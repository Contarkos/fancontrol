// Includes globaux
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "temp.h"
#include "temp_class.h"
#include "fan.h"

/* Définition des constructeurs */
TEMP::TEMP(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod) : MODULE(mod_name, m_main, m_mod)
{
    this->temp_init_pollfd();
}

TEMP::TEMP() : MODULE()
{
    this->temp_init_pollfd();
}

TEMP::~TEMP()
{
    // Pas de pointeurs à liberer
    ;
}

void TEMP::temp_init_pollfd()
{
    int ii;

    for (ii = 0; ii < TEMP_FD_NB; ii++)
    {
        this->p_fd[ii].fd = -1;
        this->p_fd[ii].events = POLLIN;
    }
}

int TEMP::start_module()
{
    int ret = 0;
    char s[] = TEMP_SOCKET_NAME;

    LOG_INF1("TEMP : Démarrage de la classe du module");

    // Démarrage du timer pour la boucle
    this->timer_fd = OS_create_timer(TEMP_TIMER_USEC, &TEMP::temp_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (0 > this->timer_fd)
    {
        LOG_ERR("TEMP : erreur création timer de boucle");
        ret = -1;
    }
    else
    {
        LOG_INF3("TEMP : timer id start = %d", timer_fd);

        // Configuration du GPIO
        ret += OS_set_gpio(TEMP_PIN_OUT, OS_GPIO_FUNC_OUT);

        // Init de l'ADC
        ret += COM_adc_init(OS_SPI_DEVICE_0, COM_ADC_CLOCK_2MHZ4);

        // Verification du setup
        ret += COM_adc_read_setup(OS_SPI_DEVICE_0, NULL);
        ret += COM_adc_read_clock(OS_SPI_DEVICE_0, NULL);

        if ( ret < 0 )
        {
            LOG_ERR("TEMP : pas de port SPI");
            this->set_running(false);
        }
    }

    // Ouverture socket UNIX
    this->socket_fd = COM_create_socket(AF_UNIX, SOCK_DGRAM, 0, s);

    if (0 == this->socket_fd)
    {
        LOG_ERR("TEMP : erreur création socket");
        ret = -2;
    }
    else
    {
        // Enregistrement de la socket
        this->p_fd[TEMP_FD_SOCKET].fd = this->socket_fd;
    }

    // Requete interruption
    this->irq_fd = OS_irq_request(OS_IRQ_ADC_NAME, O_RDONLY);

    if (0 == this->irq_fd)
    {
        LOG_ERR("TEMP : erreur requete interruption pour synchro SPI");
        ret = -4;
    }
    else
    {
        this->p_fd[TEMP_FD_IRQ].fd = this->irq_fd;
    }

    return ret;
}

int TEMP::init_after_wait(void)
{
    int ret = 0;
    char s[] = FAN_SOCKET_NAME, t[] = TEMP_SOCKET_NAME;

    LOG_INF1("TEMP : connexion en cours à la socket UNIX");

    // Connexion a la socket de FAN
    ret = COM_connect_socket(AF_UNIX, SOCK_DGRAM, s, &(this->fan_fd));

    if (0 == ret)
    {
        LOG_INF1("TEMP : connexion à la socket UNIX OK, fd = %d", this->fan_fd);

        // Connexion a la socket temp pour envoyer le message de timout
        ret = COM_connect_socket(AF_UNIX, SOCK_DGRAM, t, &(this->timeout_fd));

        if (ret != 0)
        {
            LOG_ERR("TEMP : erreur connexion au timeout timer");
        }
        else
        {
            // Démarrage du timer pour looper
            ret = OS_start_timer(this->timer_fd);

            if (ret < 0)
            {
                LOG_ERR("TEMP : erreur démarrage timer, ret = %d", ret);
            }
        }
    }

    return ret;
}

// Arret spécifique pour le module
int TEMP::stop_module()
{
    int ret = 0;

    // Arret du timer
    ret = OS_stop_timer(this->timer_fd);

    // Fermeture de la socket d'écoute
    ret += COM_close_socket(this->socket_fd);

    // Fermeture de la socket d'écoute
    ret += COM_close_socket(this->fan_fd);

    // Fermeture socket timeout
    ret += COM_close_socket(this->timeout_fd);

    // Fermeture du device SPI
    ret += OS_spi_close_port(OS_SPI_DEVICE_0);

    LOG_INF1("TEMP : fin du stop_module, ret = %d", ret);

    return ret;
}

int TEMP::exec_loop()
{
    int ret = 0;
    int read_fd = 0, ii;

    // Lecture des sockets
    read_fd = poll(this->p_fd, TEMP_FD_NB, TEMP_POLL_TIMEOUT);

    // Test de non timeout
    if (read_fd <= 0)
    {
        // Timeout expiré
        ret = 1;
    }
    else
    {
        for (ii = 0; ii < TEMP_FD_NB; ii++)
        {
            if (POLLIN & this->p_fd[ii].revents)
            {
                switch (ii)
                {
                    case TEMP_FD_SOCKET:
                        ret = temp_treat_msg();
                        break;
                    case TEMP_FD_IRQ:
                        ret = temp_treat_irq();
                        break;
                    case TEMP_FD_NB:
                    default:
                        LOG_WNG("TEMP : erreur valeur de fd, ii = %d", ii);
                        break;
                }
            }
        }
    }

    return ret;
}


