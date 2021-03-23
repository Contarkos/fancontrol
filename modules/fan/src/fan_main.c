/* Global includes */
#include <stdio.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com_msg.h"
#include "module_bis.h"

#include "fan.h"
#include "fan_module.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
t_mod_context fan_modules[] =
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

        /* Init message ID */
        .init_msg = FAN_INIT,

        /* Instance number */
        .instance_number = FAN_INSTANCE_0,
    }
};

static const t_uint32 _fan_nb_modules = sizeof(fan_modules) / sizeof (fan_modules[0]);
static       t_uint32 _fan_nb_modules_started = 0;

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Starting a FAN instance */
int FAN_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    char n[MOD_MAX_LENGTH_NAME];

    if (0 == ret)
    {
        if (_fan_nb_modules_started < _fan_nb_modules)
            LOG_INF1("%s : starting instance number %d", FAN_MODULE_NAME, _fan_nb_modules_started);
        else
        {
            LOG_ERR ("%s : no available instance, %d > %d", FAN_MODULE_NAME, _fan_nb_modules_started, _fan_nb_modules);
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Creating instance name */
        snprintf(n, MOD_MAX_LENGTH_NAME, "%s_tsk%d", FAN_MODULE_NAME, _fan_nb_modules_started);

        /* Instance configuration */
        ret = MODULE_config(&fan_modules[_fan_nb_modules_started], n, m_main, m_mod);

        if (0 != ret)
            LOG_ERR("FAN : could not configure module properly, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Thread creation */
        ret = OS_create_thread(&fan_modules[_fan_nb_modules_started].thread, (void *) &(fan_modules[_fan_nb_modules_started]));

        if (0 != ret)
            LOG_ERR("FAN : could not create thread, ret = %d", ret);
    }

    if (0 == ret)
        /* Increasing number of instances started */
        _fan_nb_modules_started++;

    return ret;
}

int FAN_stop(void)
{
    int ret = 0;

    LOG_INF1("FAN : stopping module");

    if (_fan_nb_modules_started > 0)
    {
        /* Increasing number of instances stopped */
        _fan_nb_modules_started--;

        /* Stopping execution */
        MODULE_exit( &(fan_modules[_fan_nb_modules_started]) );

        /* Reattaching thread to avoid zombies */
        ret = OS_joint_thread(&fan_modules[_fan_nb_modules_started].thread, NULL);
    }

    return ret;
}
