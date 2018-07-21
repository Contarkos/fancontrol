// Includes globaux
#include <stdio.h>

// Inlcudes locaux
#include "os.h"
#include "fan.h"
#include "fan_class.h"


// Variables globales
FAN* instances_fan[NB_INSTANCES_FAN];

// Fonctions

int FAN_start(std::mutex *m)
{
    int ret = 0, ii = 0;

    // Création de l'instance
    instances_fan[ii] = new FAN(FAN_MODULE_NAME, m);

    // Creation du thread
    OS_create_thread(instances_fan[ii]->MOD_getThread(),(void *) instances_fan[ii]);

    // Init des instances
    return ret;
}

int FAN_stop(void)
{
    int ret = 0, ii;

    for (ii = 0; ii < NB_INSTANCES_FAN; ii++)
    {
        // On coupe l'execution
        instances_fan[ii]->stop_module();

        // On réattache le thread pour éviter les zombies
        OS_joint_thread(instances_fan[ii]->MOD_getThread(), NULL);

        // Clean des instances
        instances_fan[ii]->~FAN();
    }

    return ret;
}
