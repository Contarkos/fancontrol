/* Includes globaux */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

/* Includes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

/* Variables globales */

static t_uint32 fan_msg_array[] =
{
    MAIN_START,
    MAIN_SHUTDOWN,
    FAN_MODE,
    FAN_POWER,
    FAN_TIMER,
    TEMP_DATA
};

static t_uint32 fan_msg_array_size = sizeof(fan_msg_array) / sizeof(fan_msg_array[0]);

/* Définition des constructeurs */
FAN::FAN(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod) : MODULE(mod_name, m_main, m_mod)
{
    this->fan_init_pollfd();
}

FAN::FAN() : MODULE()
{
    this->fan_init_pollfd();
}

void FAN::fan_init_pollfd()
{
    int ii;

    for (ii = 0; ii < FAN_FD_NB; ii++)
    {
        this->p_fd[ii].fd = -1;
        this->p_fd[ii].events = POLLIN;
    }
}

FAN::~FAN()
{
    ;
}

int FAN::start_module()
{
    int ret = 0;
    char s[] = FAN_SOCKET_NAME;

    LOG_INF1("FAN : Starting module class");

    /* Démarrage du timer pour la boucle */
    if (0 == ret)
    {
        this->timer_id = COM_create_timer_msg(FAN_TIMER_USEC, OS_TIMER_PERIODIC, FAN_TIMER);

        if (0 > this->timer_id)
        {
            LOG_ERR("FAN : error while creating looping timer");
            ret = -1;
        }
    }

    /* Set de la diode en entrée pour lire la vitesse */
    if (0 == ret)
    {
        ret = OS_set_gpio(FAN_PIN_IN, OS_GPIO_FUNC_IN);

        if (ret != 0)
            LOG_ERR("FAN : error setting GPIO input, ret = %d", ret);
    }

    /* Set de la pin en sortie pour le controle de l'allumage */
    if (0 == ret)
    {
        ret = OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_OUT);

        if (ret != 0)
            LOG_ERR("FAN : error setting GPIO output, ret = %d", ret);
    }

    /* Set de la diode en sortie en PWM */
    if (0 == ret)
    {
        ret = OS_set_gpio(FAN_PIN_PWM, OS_GPIO_FUNC_ALT5);

        if (ret != 0)
            LOG_ERR("FAN : error setting GPIO PWM function, ret = %d", ret);
    }

    /* Configuration of the PWM Hardware */
    if (0 == ret)
    {
        ret = fan_set_pwm();

        if (ret)
            LOG_ERR("FAN : error while setting up PWM hardware, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Creation socket */
        LOG_INF1("FAN : creating listening UNIX socket");

        /* Ouverture socket UNIX */
        this->socket_fd = COM_create_socket(AF_UNIX, SOCK_DGRAM, 0, s, sizeof(s));

        if (this->socket_fd > 0)
        {
            LOG_INF3("FAN : creation socket OK, fd = %d", this->socket_fd);
            this->p_fd[FAN_FD_SOCKET].fd = this->socket_fd;
        }
        else
        {
            LOG_ERR("FAN : error creating UNIX socket");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Request interruption */
        this->irq_fd = OS_irq_request(OS_IRQ_TIME_NAME, O_RDONLY);

        if (0 == this->irq_fd)
        {
            LOG_ERR("FAN : error requesting interruption for reading fan speed");
            ret = -4;
        }
        else
        {
            LOG_INF3("FAN : request for IRQ OK, fd = %d", this->irq_fd);
            this->p_fd[FAN_FD_IRQ].fd = this->irq_fd;
        }
    }

    if (0 == ret)
    {
        /* Register the module */
        ret = COM_msg_register(COM_ID_FAN, &this->fan_semfd);

        if (0 != ret)
        {
            LOG_ERR("FAN : error while registering the module in COM");
            ret = -8;
        }
        else
        {
            this->p_fd[FAN_FD_COM].fd = this->fan_semfd;

            ret = COM_msg_subscribe_array(COM_ID_FAN, fan_msg_array, fan_msg_array_size);

            if (ret)
                LOG_ERR("FAN : error while subscribing to messages, ret = %d", ret);
        }
    }

    LOG_INF3("FAN : end of module init (ret = %d)", ret);
    return ret;
}

/* Initialisation après l'init de tous les modules */
int FAN::init_after_wait(void)
{
    int ret = 0;

    if (0 == ret)
    {
        /* Demarrage du timer */
        ret = OS_start_timer(this->timer_id);

        if (ret < 0)
            LOG_ERR("FAN : timer not started, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Activation PWM... */
        ret = OS_pwn_enable(OS_STATE_ON);

        if (ret)
            LOG_ERR("FAN : error while configuring PWM, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Allumage du fan */
        ret = fan_set_power(FAN_POWER_MODE_ON);

        if (ret)
            LOG_ERR("FAN : error while activating fan, ret = %d", ret);
    }

    return ret;
}

/* Arret spécifique à ce module */
int FAN::stop_module()
{
    int ret = 0;

    /* Arret du ventilateur */
    {
        ret = this->fan_set_power(FAN_POWER_MODE_OFF);

        if (ret)
            LOG_ERR("FAN : error while stopping fan, ret = %d", ret);
    }

    /* TODO : Stop PWM properly */
    {
        ret = OS_pwn_enable(OS_STATE_OFF);

        if (ret)
            LOG_ERR("FAN : error while stopping PWM, ret = %d", ret);
    }

    /* Arret du timer */
    {
        ret = OS_stop_timer(this->timer_id);

        if (ret)
            LOG_ERR("FAN : error while stopping timer, ret = %d", ret);
    }

    /* Fermeture de la socket */
    {
        ret = COM_close_socket(this->socket_fd);

        if (ret)
            LOG_ERR("FAN : error while closing socket, ret = %d", ret);
    }

    /* Fermeture IRQ */
    {
        ret = OS_irq_close(this->irq_fd);

        if (ret)
            LOG_ERR("FAN : error while closing IRQ, ret = %d", ret);
    }

    LOG_INF3("FAN : end of stop_module, ret = %d", ret);
    return ret;
}

int FAN::exec_loop()
{
    int ret = 0, read_fd = 0, ii;

    /* Ecoute sur les fd */
    read_fd = poll(this->p_fd, FAN_FD_NB, FAN_POLL_TIMEOUT);

    /* Test de non timeout */
    if (read_fd <= 0)
    {
        /* Timeout expiré */
        LOG_WNG("FAN : timeout poll expired");
        ret = 1;
    }
    else
    {
        for (ii = 0; ii < FAN_FD_NB; ii++)
        {
            if (POLLIN & this->p_fd[ii].revents)
            {
                switch (ii)
                {
                    case FAN_FD_SOCKET:
                        ret = fan_treat_msg(this->p_fd[ii].fd);
                        break;
                    case FAN_FD_IRQ:
                        ret = fan_treat_irq(this->p_fd[ii].fd);
                        break;
                    case FAN_FD_COM:
                        ret = fan_treat_com();
                        break;
                    case FAN_FD_NB:
                    default:
                        LOG_WNG("FAN : bad file descriptor, index = %d", ii);
                        ret = 2;
                        break;
                }
            }
        }
    }

    return ret;
}

