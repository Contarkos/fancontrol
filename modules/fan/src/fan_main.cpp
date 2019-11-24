/* Includes globaux */
#include <stdio.h>
#include <pthread.h>

/* Inlcudes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/
FAN instances_fan[NB_INSTANCES_FAN];

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

/* Démarrage d'une instance de FAN */
int FAN_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    static int ii = 0;
    char n[MAX_LENGTH_MOD_NAME];

    if (ii < NB_INSTANCES_FAN)
    {
        /* Creation du nom de l'instance */
        snprintf(n, MAX_LENGTH_MOD_NAME, "%s_tsk%d", FAN_MODULE_NAME, ii);

        /* Création de l'instance */
        instances_fan[ii].mod_config(n, m_main, m_mod);

        /* Creation du thread */
        OS_create_thread(instances_fan[ii].MOD_getThread(),(void *) &(instances_fan[ii]));

        /* Init des instances */
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
    int ret = 0;
    static int ii = 0;

    LOG_INF1("FAN : arrêt du module");

    if (ii < NB_INSTANCES_FAN)
    {
        /* On coupe l'execution */
        MODULE::exit_module( &(instances_fan[ii]) );

        /* On réattache le thread pour éviter les zombies */
        OS_joint_thread(instances_fan[ii].MOD_getThread(), NULL);

        /* Incrementation du nombre d'instances stoppées */
        ii++;
    }

    return ret;
}
