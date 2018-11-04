// Includes globaux
#include <pthread.h>
#include <stdio.h>
#include <mutex>

// Inlcudes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module.h"
#include "temp.h"
#include "temp_class.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/
TEMP instances_temp[NB_INSTANCES_TEMP];

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

/* Démarrage d'une instance de TEMP */
int TEMP_start(std::mutex *m_main, std::mutex *m_mod)
{
    int ret = 0;
    static int ii = 0;
    char n[MAX_LENGTH_MOD_NAME];

    if (ii < NB_INSTANCES_TEMP)
    {
        // Creation du nom de l'instance
        snprintf(n, MAX_LENGTH_MOD_NAME, "%s_tsk%d", TEMP_MODULE_NAME, ii);

        // Création de l'instance
        instances_temp[ii].mod_config(n, m_main, m_mod);

        // Creation du thread
        OS_create_thread(instances_temp[ii].MOD_getThread(),(void *) &(instances_temp[ii]));

        // Init des instances
        ii++;
    }
    else
    {
        LOG_ERR("TEMP : plus d'instances disponibles, %d > %d", ii, NB_INSTANCES_TEMP);
        ret = -1;
    }

    return ret;
}

int TEMP_stop(void)
{
    int ret = 0;
    static int ii = 0;

    LOG_INF1("TEMP : arrêt du module");

    if (ii < NB_INSTANCES_TEMP)
    {
        // On coupe l'execution
        MODULE::exit_module( &(instances_temp[ii]) );

        // On réattache le thread pour éviter les zombies
        OS_joint_thread(instances_temp[ii].MOD_getThread(), NULL);

        // Incrementation du nombre d'instances stoppées
        ii++;
    }

    return ret;
}
