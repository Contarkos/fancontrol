// Includes globaux
#include <stdio.h>
#include <unistd.h>
#include <mutex>

// Includes locaux
#include "base.h"
#include "os.h"
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

    ret = OS_stop_timer(this->timer_fd);

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

void TEMP::temp_timer_handler(size_t i_timer_id, void * i_data)
{
    TEMP *p_this = reinterpret_cast<TEMP *> (i_data);

    if (p_this && (p_this->timer_fd == i_timer_id))
    {
        p_this->temp_retrieve_data();
    }
}

int TEMP::temp_retrieve_data(void)
{
    int ret = 0, ii;
    const int l = 3;
    unsigned char data[l];

    // Blablabla
    printf("[IS] TEMP : timer activé !\n");

    // Mise en forme du buffer pour lire le registre de comparaison
    data[0] = 0x00; // 0b00000000
    data[1] = 0xB0; // 0b10110000
    data[2] = 0xB0; // 0b10110000

    // On va récupérer les données
    ret = OS_spi_write_read(OS_SPI_DEVICE_0, data, l);

    if (ret < 0)
    {
        printf("[ER] TEMP : pas de données disponibles...\n");
    }
    else
    {
        // Affichage
        for (ii = 0; ii < l; ii++)
        {
            printf("[IS] TEMP : octet %d = %d\n", ii, data[ii]);
        }

        // Envoi des données à FAN via socket
    }

    return ret;
}
