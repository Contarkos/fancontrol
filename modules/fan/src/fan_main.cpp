// Includes globaux
#include <stdio.h>
#include <pthread.h>
#include <mutex>

// Inlcudes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/
FAN* instances_fan[NB_INSTANCES_FAN];

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

int FAN_start(std::mutex *m_main, std::mutex *m_mod)
{
    int ret = 0;
    static int ii = 0;

    if (ii < NB_INSTANCES_FAN)
    {
        // Création de l'instance
        instances_fan[ii] = new FAN(FAN_MODULE_NAME, m_main, m_mod);

        // Creation du thread
        OS_create_thread(instances_fan[ii]->MOD_getThread(),(void *) instances_fan[ii]);

        // Init des instances
        ii++;
    }
    else
    {
        LOG_ERR("FAN : plus d'instances disponibles, %d > %d", ii, NB_INSTANCES_FAN);
        ret = -1;
    }
    return ret;
}

int FAN_stop(void)
{
    int ret = 0, ii;

    LOG_INF1("FAN : arrêt du module");

    for (ii = 0; ii < NB_INSTANCES_FAN; ii++)
    {
        // On coupe l'execution
        MODULE::exit_module(instances_fan[ii]);

        // On réattache le thread pour éviter les zombies
        OS_joint_thread(instances_fan[ii]->MOD_getThread(), NULL);

        // Clean des instances
        instances_fan[ii]->~FAN();
    }

    return ret;
}
