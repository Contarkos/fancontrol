// Includes globaux
#include <pthread.h>
#include <stdio.h>
#include <mutex>

// Inlcudes locaux
#include "base.h"
#include "os.h"
#include "module.h"
#include "temp.h"
#include "temp_class.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/
TEMP* instances_temp[NB_INSTANCES_TEMP];

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

int TEMP_start(std::mutex *m_main, std::mutex *m_mod)
{
    int ret = 0;
    static int ii = 0;

    if (ii < NB_INSTANCES_TEMP)
    {
        // Création de l'instance
        instances_temp[ii] = new TEMP(TEMP_MODULE_NAME, m_main, m_mod);

        // Creation du thread
        OS_create_thread(instances_temp[ii]->MOD_getThread(),(void *) instances_temp[ii]);

        // Init des instances
        ii++;
    }
    else
    {
        printf("[ER] TEMP : plus d'instances disponibles, %d > %d\n", ii, NB_INSTANCES_TEMP);
        ret = -1;
    }

    return ret;
}

int TEMP_stop(void)
{
    int ret = 0, ii;

    for (ii = 0; ii < NB_INSTANCES_TEMP; ii++)
    {
        // On coupe l'execution
        MODULE::exit_module(instances_temp[ii]);

        // On réattache le thread pour éviter les zombies
        OS_joint_thread(instances_temp[ii]->MOD_getThread(), NULL);

        // Clean des instances
        instances_temp[ii]->~TEMP();
    }

    return ret;
}
