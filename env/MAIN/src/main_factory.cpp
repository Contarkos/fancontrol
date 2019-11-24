/* Includes globaux */

/* Includes locaux */
#include "base.h"
#include "base_typ.h"
#include "integ_log.h"
#include "os.h"
#include "main_factory.h"
#include "module.h"
#include "fan.h"
#include "temp.h"
#include "remote.h"
#include "cmd.h"
#include "com.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "shmd.h"

#ifdef __cplusplus
}
#endif

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

int main_is_running = 0;

mod_type t_start[NB_MODULE] = {
        {&FAN_start, &FAN_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
        {&TEMP_start, &TEMP_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
        {&REMOTE_start, &REMOTE_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
        {&SHMD_start, &SHMD_stop, OS_INIT_MUTEX, OS_INIT_MUTEX}
};

/*********************************************************************/
/*                       Fonctions internes                          */
/*********************************************************************/

int main_start_factory()
{
    int ret, ret_temp, ii;

    ret = 0;

    /* Init des librairies */
    ret_temp = main_init();

    if (0 != ret_temp)
    {
        ret = 1;
    }
    else
    {
        /* On démarre les modules. */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            /* On bloque les mutex de tout le monde */
            LOG_INF1("MAIN : lock pour %d", ii);
            OS_mutex_lock(&(t_start[ii].mutex_mod));
            OS_mutex_unlock(&(t_start[ii].mutex_main));

            ret_temp = t_start[ii].mod_start(&(t_start[ii].mutex_main), &(t_start[ii].mutex_mod));

            if (0 != ret_temp)
            {
                ret = 1;
                LOG_ERR("MAIN : Erreur pendant le lancement du module n°%d", ii);
            }
        }

        /* Attente du retour des threads du module */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            LOG_INF1("MAIN : lock d'attente pour %d", ii);
            OS_mutex_lock(&(t_start[ii].mutex_main));
        }

        /* Tous les threads sont lancés on peut démarrer */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            LOG_INF1("MAIN : unlock pour %d", ii);
            OS_mutex_unlock(&(t_start[ii].mutex_mod));
        }

        /* C'est parti */
        main_is_running = 1;

        /* On va chercher des commandes rentrées par l'utilisateur */
        ret = main_loop();
    }

    return ret;
}

int main_init(void)
{
    int ret = 0;

    /* Init de l'OS */
    ret = OS_init();

    /* Init de COM */
    if (ret != 0)
    {
        LOG_ERR("MAIN : erreur à l'init de l'OS, code : %d", ret);
    }
    else
    {
        /* Init de la socket UDP */
        if (0 != (ret = COM_init()))
        {
            LOG_ERR("MAIN : erreur à l'init de COM, code : %d", ret);
        }
        /* Init des regex de parsing des commandes */
        else if (0 != (ret = CMD_init()))
        {
            LOG_ERR("MAIN : erreur à l'init de CMD, code : %d", ret);
        }
        else
        {
            LOG_INF1("MAIN : init du systeme OK");
        }

    }

    return ret;
}

int main_loop(void)
{
    int ret = 0;

    while (main_is_running)
    {
        CMD_read();
    }

    return ret;
}

int main_stop_factory()
{
    int ret = 0, ii;
    static int is_called = 0;

    LOG_INF1("MAIN : extinction des modules");

    /* Pour éviter les arrets multiples */
    if (is_called)
    {
        LOG_WNG("MAIN : Appel d'arret déjà effectué pour factory");
        ret = 1;
    }
    else
    {
        is_called = 1;

        /* On lance les demandes d'arrets */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            /* TODO envoi des messages MAIN_SHUTDOWN */
            t_start[ii].mod_stop();
        }

        LOG_INF1("MAIN : Attente des locks à libérer pour les modules");

        /* On attend que tous les threads soient terminés */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            OS_mutex_lock(&(t_start[ii].mutex_mod));
        }

        /* Arret des éléments du système */
        LOG_INF1("MAIN : Arret des modules systemes");
        ret += main_stop();
    }

    return ret;
}

int main_stop(void)
{
    int ret = 0;

    /* Arret de la COM exterieure */
    ret += COM_stop();

    /* Arret du parsing des regex */
    ret += CMD_stop();

    /* Arret de l'OS */
    ret += OS_stop();

    return ret;
}

