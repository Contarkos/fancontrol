// Includes globaux
#include <stdio.h>

// Includes locaux
#include "module.h"
#include "fan_class.h"

/* Définition des constructeurs */
FAN::FAN(char *mod_name, std::mutex *m) : MODULE(mod_name, m)
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

    return ret;
}

int FAN::exec_loop()
{
    static int n = 0;

    printf("FAN : loop n°%d!\n", n);
    n++;

    return 0;
}
