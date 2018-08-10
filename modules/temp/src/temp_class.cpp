// Includes globaux
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

/* Définition des constructeurs */
TEMP::TEMP(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m) : MODULE(mod_name, m)
{
    ;
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
    this->timer_fd += OS_create_timer(TEMP_TIMER_USEC, &TEMP::temp_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (0 == this->timer_fd)
    {
        printf("[ER] TEMP : erreur création timer de boucle\n");
        ret = -1;
    }
    else
    {
        // Configuration du GPIO
        ret += OS_set_gpio(TEMP_PIN_OUT, OS_GPIO_FUNC_IN);

        // Configuration du module SPI
        ret = OS_spi_open_port(OS_SPI_DEVICE_0);

        if ( ret < 0 )
        {
            printf("[ER] TEMP : pas de port SPI\n");
            this->set_running(false);
        }
        else
        {
            // Démarrage du timer pour looper
            ret = OS_start_timer(this->timer_fd);
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

    // Fermeture de la socket
    // ret += COM_socket_close(this->socket_fd);

    // Fermeture du device SPI
    ret += OS_spi_close_port(OS_SPI_DEVICE_0);

    return ret;
}

int TEMP::exec_loop()
{
    int ret = 0;
    static int n = 0;
    const int max = 1000;

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

    return ret;
}


