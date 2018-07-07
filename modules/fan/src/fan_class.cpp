// Includes globaux
#include <stdio.h>

// Includes locaux
#include "module.h"
#include "fan_class.h"

/* Définition des constructeurs */
FAN::FAN(char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m) : MODULE(mod_name, m)
{
    ;
}

FAN::~FAN()
{
    ;
}

int FAN::start_module()
{
    int ret = 0;

    printf("Démarrage de la classe du module\n");

    return ret;
}

int FAN::stop_module()
{
    int ret = 0;

    // Arret de la boucle
    this->set_running(false);

    return ret;
}

int FAN::exec_loop()
{
    static int n = 0;
    const int max = 100;

    printf("FAN : loop n°%d!\n", n);

    if (n > max)
    {
        this->set_running(false);
    }
    else
    {
        n++;
    }

    return 0;
}
