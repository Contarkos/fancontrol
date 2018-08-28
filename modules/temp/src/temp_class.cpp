// Includes globaux
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <math.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "temp_class.h"
#include "fan.h"

/* Définition des constructeurs */
TEMP::TEMP(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod) : MODULE(mod_name, m_main, m_mod)
{
    int ii;

    for (ii = 0; ii < TEMP_FD_NB; ii++)
    {
        this->p_fd[ii].fd = -1;
        this->p_fd[ii].events = POLLIN;
    }
}

TEMP::~TEMP()
{
    ;
}

int TEMP::start_module()
{
    int ret = 0;

    printf("[IS] TEMP : Démarrage de la classe du module\n");

    // Démarrage du timer pour la boucle
    this->timer_fd = OS_create_timer(TEMP_TIMER_USEC, &TEMP::temp_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (0 == this->timer_fd)
    {
        printf("[ER] TEMP : erreur création timer de boucle\n");
        ret = -1;
    }
    else
    {
        printf("[IS] TEMP : timer_fd start = %x\n", timer_fd);

        // Configuration du GPIO
        ret += OS_set_gpio(TEMP_PIN_OUT, OS_GPIO_FUNC_IN);

        // Configuration du module SPI
        ret += OS_spi_open_port(OS_SPI_DEVICE_0);

        if ( ret < 0 )
        {
            printf("[ER] TEMP : pas de port SPI\n");
            this->set_running(false);
        }
        else
        {
            // Démarrage du timer pour looper
            ret = OS_start_timer(this->timer_fd);

            if (ret < 0)
            {
                printf("[ER] TEMP : erreur démarrage timer, ret = %d\n", ret);
            }
        }
    }

    return ret;
}

int TEMP::init_after_wait(void)
{
    int ret = 0;

    printf("[IS] TEMP : connexion en cours à la socket UNIX\n");

    // Connexion a la socket de FAN
    this->fan_fd = g_fan_socket;
    //ret = COM_connect_socket(AF_UNIX, SOCK_STREAM, s, &(this->fan_fd));

    if (0 == ret)
    {
        printf("[IS] TEMP : connexion à la socket UNIX OK, fd = %d\n", this->fan_fd);
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

    // Fermeture du device SPI
    ret += OS_spi_close_port(OS_SPI_DEVICE_0);

    return ret;
}

int TEMP::exec_loop()
{
    int ret = 0;

#if 1
    int read_fd = 0, ii, ss;
    t_com_msg m;

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
            if (this->p_fd[ii].revents & this->p_fd[ii].events)
            {
                ss = read(this->p_fd[ii].fd, &m, sizeof(t_com_msg)); // traitement

                if (sizeof(t_com_msg) != ss)
                {
                    printf("[WG] FAN : mauvaise taille de message pour fd %d\n", ii);
                    ret = -1;
                }
                else
                {
                    ret = temp_treat_msg(m);
                }
            }
        }
    }
#else
    int n = 0, max= 1000;

    // Condition de sortie
    if (n > max)
    {
        printf("[IS] TEMP : fin du module\n");
        this->set_running(false);
    }
    else
    {
        n++;
    }

    if ( !(n % 100) )
    {
        printf("[IS] TEMP : alive !\n");
    }

    OS_usleep(100000);
#endif
    return ret;
}


