/* Global includes */
#include <stdio.h>
#include <pthread.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module_bis.h"

#include "fan.h"
#include "fan_module.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/
t_mod_context fan_modules[NB_INSTANCES_FAN] =
{
    /* Only module of FAN */
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
        .start_module = &fan_start_module,
        .stop_module  = &fan_stop_module,
        .init_after_wait = &fan_init_after_wait,
        .exec_loop    = &fan_exec_loop,
    }
};

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Starting a FAN instance */
int FAN_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    static int ii = 0;
    char n[MOD_MAX_LENGTH_NAME];

    if (ii < NB_INSTANCES_FAN)
    {
        /* Creating instance name */
        snprintf(n, MOD_MAX_LENGTH_NAME, "%s_tsk%d", FAN_MODULE_NAME, ii);

        /* Instance configuration */
        MODULE_config(&fan_modules[ii], n, m_main, m_mod);

        /* Thread creation */
        OS_create_thread(&fan_modules[ii].thread, (void *) &(fan_modules[ii]));

        /* Increasing number of instances started */
        ii++;
    }
    else
    {
        LOG_ERR("FAN : no available instance, %d > %d", ii, NB_INSTANCES_FAN);
        ret = -1;
    }

    return ret;
}

int FAN_stop(void)
{
    int ret = 0;
    static int ii = 0;

    LOG_INF1("FAN : stopping module");

    if (ii < NB_INSTANCES_FAN)
    {
        /* Stopping execution */
        MODULE_exit( &(fan_modules[ii]) );

        /* Reattaching thread to avoid zombies */
        OS_joint_thread(&fan_modules[ii].thread, NULL);

        /* Increasing number of instances stopped */
        ii++;
    }

    return ret;
}
