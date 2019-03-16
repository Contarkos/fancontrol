// Includes globaux
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

// Variables globales

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

    LOG_INF1("FAN : Démarrage de la classe du module");

    // Démarrage du timer pour la boucle
    this->timer_fd = OS_create_timer(FAN_TIMER_USEC, &FAN::fan_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (0 > this->timer_fd)
    {
        LOG_ERR("FAN : erreur création timer de boucle");
        ret = -1;
    }
    else
    {
        // Set de la diode en entrée pour lire la vitesse
        ret += OS_set_gpio(FAN_PIN_IN, OS_GPIO_FUNC_IN);

        // Set de la pin en sortie pour le controle de l'allumage
        ret += OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_OUT);

        // Set de la diode en sortie en PWM
        ret += OS_set_gpio(FAN_PIN_PWM, OS_GPIO_FUNC_ALT5);

        // Reglage source Clock sur PLL C
        ret += OS_pwm_set_clock_source(OS_CLOCK_SRC_PLLC);

        // Mode MS pour le PWM
        ret += OS_pwm_set_mode(OS_PWM_MODE_MSMODE);

        // Cycle par défaut : 1024
        ret += OS_pwm_set_precision(FAN_DEFAULT_PREC);

        // Reglage du filter MASH par défaut
        ret += OS_pwm_set_mash(OS_PWM_MASH_FILTER_1);

        // Reglage de la fréquence PWM
        ret += OS_pwm_set_frequency(FAN_PWM_FREQ);

        // Reglage du duty cycle par defaut : 50%
        ret += OS_pwm_set_dutycycle(FAN_DEFAULT_CYCLE);

        // Activation...
        ret += OS_pwn_enable(OS_STATE_ON);

        if (ret < 0)
        {
            LOG_ERR("FAN : erreur lors du parametrage du PWM, ret = %d", ret);
            return ret;
        }
        else
        {
            // Creation socket
            LOG_INF1("FAN : création de la socket d'écoute");

            // Ouverture socket UNIX
            this->socket_fd = COM_create_socket(AF_UNIX, SOCK_DGRAM, 0, s);

            if (this->socket_fd > 0)
            {
                LOG_INF3("FAN : creation socket OK, fd = %d", this->socket_fd);
                this->p_fd[FAN_FD_SOCKET].fd = this->socket_fd;
            }
            else
            {
                LOG_ERR("FAN : erreur à la création de la socket");
                ret = -1;
            }
        }
    }

    if (0 == ret)
    {
        // Requete interruption
        this->irq_fd = OS_irq_request(OS_IRQ_TIME_NAME, O_RDONLY);

        if (0 == this->irq_fd)
        {
            LOG_ERR("FAN : erreur requete interruption pour lecture vitesse fan");
            ret = -4;
        }
        else
        {
            this->p_fd[FAN_FD_IRQ].fd = this->irq_fd;
        }
    }

    LOG_INF3("FAN : fin init module (ret = %d)", ret);
    return ret;
}

// Initialisation après l'init de tous les modules
int FAN::init_after_wait(void)
{
    int ret = 0;
    char t[] = FAN_SOCKET_NAME;

    // Connexion a la socket temp pour envoyer le message de timer
    ret = COM_connect_socket(AF_UNIX, SOCK_DGRAM, t, &(this->timeout_fd));

    if (ret != 0)
    {
        LOG_ERR("FAN : error while connecting timeout socket, ret = %d", ret);
    }
    else
    {
        // Demarrage du timer
        ret = OS_start_timer(this->timer_fd);

        if (ret < 0)
        {
            LOG_ERR("FAN : timer non démarré, ret = %d", ret);
        }
        else
        {
            t_fan_power_mode m;

            // Allumage du fan
            m.power_mode = FAN_POWER_MODE_ON;
            this->fan_update_power(&m);
        }
    }

    return ret;
}

// Arret spécifique à ce module
int FAN::stop_module()
{
    int ret = 0;
    t_fan_power_mode m;

    // Arret du ventilateur
    m.power_mode = FAN_POWER_MODE_OFF;
    ret += this->fan_update_power(&m);

    // Arret du timer
    ret += OS_stop_timer(this->timer_fd);

    // Fermeture de la socket
    ret += COM_close_socket(this->socket_fd);

    // Fermeture socket timeout
    ret += COM_close_socket(this->timeout_fd);

    // Fermeture IRQ
    ret += OS_irq_close(this->irq_fd);

    LOG_INF3("FAN : fin du stop_module, ret = %d", ret);
    return ret;
}

int FAN::exec_loop()
{
    int ret = 0, read_fd = 0, ii;

    // Ecoute sur les fd
    read_fd = poll(this->p_fd, FAN_FD_NB, FAN_POLL_TIMEOUT);

    // Test de non timeout
    if (read_fd <= 0)
    {
        // Timeout expiré
        LOG_WNG("FAN : timeout poll expiré");
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
                    case FAN_FD_NB:
                    default:
                        LOG_WNG("FAN : mauvais file descriptor");
                        ret = 2;
                        break;
                }
            }
        }
    }

    return ret;
}

