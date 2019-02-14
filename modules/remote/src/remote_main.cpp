// Includes globaux
#include <stdio.h>
#include <pthread.h>
#include <mutex>

// Inlcudes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module.h"
#include "remote.h"
#include "remote_class.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/
REMOTE instances_remote[NB_INSTANCES_REMOTE];

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

/* Démarrage d'une instance de FAN */
int REMOTE_start(std::mutex *m_main, std::mutex *m_mod)
{
    int ret = 0;
    static int ii = 0;
    char n[MAX_LENGTH_MOD_NAME];

    if (ii < NB_INSTANCES_REMOTE)
    {
        // Creation du nom de l'instance
        snprintf(n, MAX_LENGTH_MOD_NAME, "%s_tsk%d", REMOTE_MODULE_NAME, ii);

        // Création de l'instance
        instances_remote[ii].mod_config(n, m_main, m_mod);

        // Creation du thread
        OS_create_thread(instances_remote[ii].MOD_getThread(),(void *) &(instances_remote[ii]));

        // Init des instances
        ii++;
    }
    else
    {
        LOG_ERR("FAN : plus d'instances disponibles, %d > %d", ii, NB_INSTANCES_REMOTE);
        ret = -1;
    }

    return ret;
}

int REMOTE_stop(void)
{
    int ret = 0;
    static int ii = 0;

    LOG_INF1("FAN : arrêt du module");

    if (ii < NB_INSTANCES_REMOTE)
    {
        // On coupe l'execution
        MODULE::exit_module( &(instances_remote[ii]) );

        // On réattache le thread pour éviter les zombies
        OS_joint_thread(instances_remote[ii].MOD_getThread(), NULL);

        // Incrementation du nombre d'instances stoppées
        ii++;
    }

    return ret;
}
