/* Global includes */
#include <stdio.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module_bis.h"

#include "remote.h"
#include "remote_module.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
t_mod_context remote_modules[NB_INSTANCES_REMOTE] =
{
    /* Only module of REMOTE */
    {
        /* Mutexes */
        .mutex_mod = NULL,
        .mutex_main = NULL,

        /* Run handling variables */
        .is_running = BASE_FALSE,
        .is_init = BASE_FALSE,

        /* Generic functions */
        .init_module = &MODULE_init,
        .exit_module = &MODULE_exit,

        .wait_and_loop = &MODULE_wait_and_loop,
        .stop_and_exit = &MODULE_stop_and_exit,

        /* Specific functions */
        .start_module = &remote_start_module,
        .stop_module  = &remote_stop_module,
        .init_after_wait = &remote_init_after_wait,
        .exec_loop    = &remote_exec_loop,
    }
};

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Starting a REMOTE instance */
int REMOTE_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    static int ii = 0;
    char n[MOD_MAX_LENGTH_NAME];

    if (ii < NB_INSTANCES_REMOTE)
    {
        /* Creating instance name */
        snprintf(n, MOD_MAX_LENGTH_NAME, "%s_tsk%d", REMOTE_MODULE_NAME, ii);

        /* Instance configuration */
        MODULE_config(&remote_modules[ii], n, m_main, m_mod);

        /* Thread creation */
        OS_create_thread(&remote_modules[ii].thread, (void *) &(remote_modules[ii]));

        /* Increasing number of instances started */
        ii++;
    }
    else
    {
        LOG_ERR("REMOTE : no instance available, %d > %d", ii, NB_INSTANCES_REMOTE);
        ret = -1;
    }

    return ret;
}

int REMOTE_stop(void)
{
    int ret = 0;
    static int ii = 0;

    LOG_INF1("REMOTE : stopping module");

    if (ii < NB_INSTANCES_REMOTE)
    {
        /* Stopping execution */
        MODULE_exit( &(remote_modules[ii]) );

        /* Reattaching thread to avoid zombies */
        OS_joint_thread(&remote_modules[ii].thread, NULL);

        /* Increasing number of instances stopped */
        ii++;
    }

    return ret;
}

