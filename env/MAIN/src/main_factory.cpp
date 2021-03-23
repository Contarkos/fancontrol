/* Global includes */
#include <string.h>

/* Local includes */
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

mod_type t_start[] = {
        {&FAN_start, &FAN_stop, OS_INIT_MUTEX, OS_INIT_MUTEX, FAN_INIT},
        {&TEMP_start, &TEMP_stop, OS_INIT_MUTEX, OS_INIT_MUTEX, TEMP_INIT},
        {&REMOTE_start, &REMOTE_stop, OS_INIT_MUTEX, OS_INIT_MUTEX, REMOTE_INIT},
        {&SHMD_start, &SHMD_stop, OS_INIT_MUTEX, OS_INIT_MUTEX, SHMD_INIT},
};

static const int _main_nb_module = sizeof(t_start) / sizeof (t_start[0]);

static t_uint32 main_msg_array[] =
{
    MAIN_SHUTDOWN,
    FAN_INIT,
    TEMP_INIT,
    SHMD_INIT,
    REMOTE_INIT
};

static t_uint32 main_msg_array_size = sizeof(main_msg_array) / sizeof(main_msg_array[0]);

static int main_sem_fd = -1;

/*********************************************************************/
/*                       Static prototypes                           */
/*********************************************************************/

static int _main_init_env   (void);
static int _main_init       (void);
static int _main_wait_modules (void);

static int _main_stop       (void);

/*********************************************************************/
/*                       Fonctions internes                          */
/*********************************************************************/

int main_start_factory()
{
    int ret, ret_temp, ii;
    int init_status = 0;

    ret = 0;
    ret_temp = 0;

    /* Init librairies */
    if (0 == ret)
    {
        ret_temp = _main_init_env();

        if (ret_temp != 0)
        {
            LOG_ERR("MAIN : error while loading environment, ret = %d", ret_temp);
            ret = 1;
        }
    }

    /* Init of main */
    if (0 == ret_temp)
    {
        ret_temp = _main_init();

        if (ret_temp != 0)
        {
            LOG_ERR("MAIN : could not init MAIN");
            ret = -1;
        }
    }

    if (0 == ret_temp)
    {
        /* Starting modules. */
        for (ii = 0; ii < _main_nb_module; ii++)
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
        for (ii = 0; ii < _main_nb_module; ii++)
        {
            LOG_INF1("MAIN : locking on %d", ii);
            OS_mutex_lock(&(t_start[ii].mutex_main));
        }

        /* Check status for every modules */
        init_status = _main_wait_modules();

        /* All modules are ready so : GO ! */
        for (ii = 0; ii < _main_nb_module; ii++)
        {
            LOG_INF1("MAIN : unlock for %d", ii);
            OS_mutex_unlock(&(t_start[ii].mutex_mod));
        }

        if (0 == ret)
        {
            /* TODO : send MAIN_START message to start everyone */
            ret = COM_msg_send(MAIN_START, &init_status, sizeof(init_status));
        }

        /* Main is now running */
        main_is_running = 1;

        /* Main loop will wait for commands from the user */
        ret = main_loop();
    }

    return ret;
}

static int _main_init_env(void)
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

static int _main_init(void)
{
    int ret = 0;

    if (0 == ret)
    {
        ret = COM_msg_register(COM_ID_MAIN, &main_sem_fd);

        if (ret != 0)
            LOG_ERR("MAIN : error while registering for messages");
    }

    if (0 == ret)
    {
        ret = COM_msg_subscribe_array(COM_ID_MAIN, main_msg_array, main_msg_array_size);

        if (ret != 0)
            LOG_ERR("MAIN : error while subscribing to messages");
    }

    return ret;
}

static int _main_wait_modules(void)
{
    int init_status = 0;
    int ret, ii, jj;
    t_com_msg_struct msg;
    t_com_msg_init result;
    int status[_main_nb_module] = { 0 };

    ii = 0;

    /* TODO : add a timer for a timeout */
    while (ii < _main_nb_module)
    {
        /* Wait for messages */
        ret = COM_msg_read(COM_ID_MAIN, &msg);

        if (ret != 0)
        {
            LOG_ERR("MAIN : error receiving status from modules");
            init_status = -1;
            break;
        }
        else
        {
            for (jj = 0; jj < _main_nb_module; jj++)
            {
                /* We need to find a module with the same ID and no status returned yet */
                if ( (msg.header.id == t_start[jj].msg_id) && (0 == status[jj]) )
                {
                    memcpy(&result, msg.body, sizeof(result));

                    status[jj] = result.status;
                    ii++;
                    break;
                }
            }
        }
    }

    for (ii = 0; ii < _main_nb_module; ii++)
    {
        if (status[ii] != 1)
            init_status = -1;
    }

    return init_status;
}

int main_loop(void)
{
    int ret = 0;

    while (main_is_running)
        CMD_read();

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
        for (ii = 0; ii < _main_nb_module; ii++)
            /* TODO envoi des messages MAIN_SHUTDOWN */
            t_start[ii].mod_stop();

        LOG_INF1("MAIN : Waiting on locks for the modules");

        /* On attend que tous les threads soient terminés */
        for (ii = 0; ii < _main_nb_module; ii++)
            OS_mutex_lock(&(t_start[ii].mutex_mod));

        /* Arret des éléments du système */
        LOG_INF1("MAIN : Stopping system modules");
        ret += _main_stop();
    }

    return ret;
}

static int _main_stop(void)
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

