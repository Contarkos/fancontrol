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
#include "shmd.h"
#include "cmd.h"
#include "com.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

int main_is_running = 0;

mod_type t_start[NB_MODULE] = {
        {&FAN_start, &FAN_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
        {&TEMP_start, &TEMP_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
        {&REMOTE_start, &REMOTE_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
        {&SHMD_start, &SHMD_stop, OS_INIT_MUTEX, OS_INIT_MUTEX},
};

/*********************************************************************/
/*                       Fonctions internes                          */
/*********************************************************************/

int main_start_factory()
{
    int ret, ret_temp, ii;

    ret = 0;

    /* Init librairies */
    ret_temp = main_init();

    if (0 != ret_temp)
    {
        LOG_ERR("MAIN : error while loading environment, ret = %d", ret_temp);
        ret = 1;
    }
    else
    {
        /* Starting modules. */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            /* Locking mutexes for everyone */
            LOG_INF1("MAIN : lock for %d", ii);
            OS_mutex_lock(&(t_start[ii].mutex_mod));
            OS_mutex_unlock(&(t_start[ii].mutex_main));

            ret_temp = t_start[ii].mod_start(&(t_start[ii].mutex_main), &(t_start[ii].mutex_mod));

            if (0 != ret_temp)
            {
                ret = 1;
                LOG_ERR("MAIN : Error while launching module n°%d", ii);
            }
        }

        /* Waiting on modules to be initialised */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            LOG_INF1("MAIN : locking on %d", ii);
            OS_mutex_lock(&(t_start[ii].mutex_main));
        }

        /* All modules are ready so : GO ! */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            LOG_INF1("MAIN : unlock for %d", ii);
            OS_mutex_unlock(&(t_start[ii].mutex_mod));
        }

        if (0 == ret)
        {
            /* TODO : send MAIN_START message to start everyone */
            int dummy = 0;
            ret = COM_msg_send(MAIN_START, &dummy, sizeof(dummy));
        }

        /* Main is now running */
        main_is_running = 1;

        /* Main loop will wait for commands from the user */
        ret = main_loop();
    }

    return ret;
}

int main_init(void)
{
    int ret = 0;

    if (0 == ret)
    {
        /* Init de l'OS */
        ret = OS_init();

        if (ret != 0)
            LOG_ERR("MAIN : error on init for OS, code : %d", ret);
    }

    if (0 == ret)
    {
        /* Init de COM */
        ret = COM_init();

        if (ret)
            LOG_ERR("MAIN : error on init for COM, code : %d", ret);
    }

    if (0 == ret)
    {
        /* Init des regex de parsing des commandes */
        ret = CMD_init();

        if (ret)
            LOG_ERR("MAIN : error on init for CMD, code : %d", ret);
    }

    if (0 == ret)
        LOG_INF1("MAIN : init system OK");

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

    LOG_INF1("MAIN : stopping all modules");

    /* Pour éviter les arrets multiples */
    if (is_called)
    {
        LOG_WNG("MAIN : Stopping call already done for factory");
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

        LOG_INF1("MAIN : Waiting on locks for the modules");

        /* On attend que tous les threads soient terminés */
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            OS_mutex_lock(&(t_start[ii].mutex_mod));
        }

        /* Arret des éléments du système */
        LOG_INF1("MAIN : Stopping system modules");
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

