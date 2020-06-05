/* Global includes */
#include <pthread.h>
#include <stdio.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module_bis.h"

#include "temp.h"
#include "temp_module.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
t_mod_context temp_modules[NB_INSTANCES_TEMP] =
{
    /* Only module of TEMP */
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
        .start_module = &temp_start_module,
        .stop_module  = &temp_stop_module,
        .init_after_wait = &temp_init_after_wait,
        .exec_loop    = &temp_exec_loop,
    }
};

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Starting a TEMP instance */
int TEMP_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    static int ii = 0;
    char n[MOD_MAX_LENGTH_NAME];

    if (ii < NB_INSTANCES_TEMP)
    {
        /* Creating instance name */
        snprintf(n, MOD_MAX_LENGTH_NAME, "%s_tsk%d", TEMP_MODULE_NAME, ii);

        /* Instance configuration */
        MODULE_config(&temp_modules[ii], n, m_main, m_mod);

        /* Thread creation */
        OS_create_thread(&temp_modules[ii].thread, (void *) &(temp_modules[ii]));

        /* Increasing number of instances started */
        ii++;
    }
    else
    {
        LOG_ERR("TEMP : no available instance, %d > %d", ii, NB_INSTANCES_TEMP);
        ret = -1;
    }

    return ret;
}

int TEMP_stop(void)
{
    int ret = 0;
    static int ii = 0;

    LOG_INF1("TEMP : stopping module");

    if (ii < NB_INSTANCES_TEMP)
    {
        /* Stopping execution */
        MODULE_exit( &(temp_modules[ii]) );

        /* Reattaching thread to avoid zombies */
        OS_joint_thread(&temp_modules[ii].thread, NULL);

        /* Increasing number of instances stopped */
        ii++;
    }

    return ret;
}
