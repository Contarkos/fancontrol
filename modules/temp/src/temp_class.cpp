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
    ret += OS_create_timer();

    if (0 != ret)
    {
        printf("[ER] TEMP : erreur création timer de boucle\n");
    }
    else
    {
        // Configuration du module SPI
    }

    return ret;
}

// Arret spécifique pour le module
int TEMP::stop_module()
{
    int ret = 0;

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

    if ( n % 10 )
    {
        printf("[IS] TEMP : alive !\n");
    }

    OS_usleep(100000);

    return ret;
}
