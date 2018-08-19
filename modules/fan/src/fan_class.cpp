// Includes globaux
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <poll.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

/* Définition des constructeurs */
FAN::FAN(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m) : MODULE(mod_name, m)
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

    printf("[IS] FAN : Démarrage de la classe du module\n");

    // Démarrage du timer pour la boucle
    this->timer_fd = OS_create_timer(FAN_TIMER_USEC, &FAN::fan_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (0 == this->timer_fd)
    {
        printf("[ER] FAN : erreur création timer de boucle\n");
        ret = -1;
    }
    else
    {
        // Set de la diode en entrée pour lire la vitesse
        ret += OS_set_gpio(FAN_PIN_IN, OS_GPIO_FUNC_IN);

        // Reglage source Clock sur PLL C
        ret += OS_pwm_set_clock_source(OS_CLOCK_SRC_PLLC);

        // Set de la diode en sortie en PWM
        ret += OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_ALT5);

        // Mode MS pour le PWM
        ret += OS_pwm_set_mode(OS_PWM_MODE_MSMODE);

        // Cycle par défaut : 1024
        ret += OS_pwm_set_precision(FAN_DEFAULT_PREC);

        // Reglage de la fréquence PWM
        ret += OS_pwm_set_frequency(FAN_PWM_FREQ);

        // Reglage du duty cycle par defaut : 50%
        ret += OS_pwm_set_dutycycle(FAN_DEFAULT_CYCLE);

        // Activation...
        ret += OS_pwn_enable(OS_STATE_ON);

        if (ret >= 0)
        {
            return ret;
        }
        else
        {
            // Demarrage du timer
            //ret = OS_start_timer(this->timer_fd);

            // Ouverture socket UNIX
            this->socket_fd = COM_create_socket(AF_UNIX, SOCK_STREAM, 0, s);

            if (this->socket_fd >= 0)
            {
                this->p_fd[FAN_FD_SOCKET].fd = this->socket_fd;
            }
        }
    }

    return ret;
}

// Initialisation après l'init de tous les modules
int FAN::init_after_wait(void)
{
    int ret = 0;

    return ret;
}

// Arret spécifique à ce module
int FAN::stop_module()
{
    int ret = 0;

    // Arret du timer
    ret = OS_stop_timer(this->timer_fd);

    // Fermeture de la socket
    ret = COM_close_socket(this->socket_fd);

    return ret;
}

int FAN::exec_loop()
{
    int ret = 0, read_fd = 0, ii, ss;
    static int n = 0;
    static int is_onoff = 0;
    const int max = 100;
    t_com_msg m;

#if 1
    // Ecoute sur les fd
    read_fd = poll(this->p_fd, FAN_FD_NB, FAN_POLL_TIMEOUT);

    // Test de non timeout
    if (read_fd <= 0)
    {
        // Timeout expiré
        ret = 1;
    }
    else
    {
        for (ii = 0; ii < FAN_FD_NB; ii++)
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
                    ret = fan_treat_msg(m);
                }
            }
        }
    }
#else
    if (is_onoff)
    {
        OS_pwm_set_dutycycle((float) 0);
    }
    else
    {
        OS_pwm_set_dutycycle((float) 100);
    }

    usleep(10000000);
#endif
    is_onoff = 1 - is_onoff;

    // Condition de sortie
    if (n > max)
    {
        printf("[IS] FAN : fin du module\n");
        this->set_running(false);
    }
    else
    {
        n++;
    }

    return ret;
}

