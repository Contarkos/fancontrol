/* Global includes */
#include <stdio.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com_msg.h"
#include "module_bis.h"

#include "temp.h"
#include "temp_module.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
t_mod_context temp_modules[] =
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

        /* Init message ID */
        .init_msg = TEMP_INIT,

        /* Instance number */
        .instance_number = TEMP_INSTANCE_0,
    }
};

static const t_uint32 _temp_nb_modules = sizeof(temp_modules) / sizeof (temp_modules[0]);
static       t_uint32 _temp_nb_modules_started = 0;

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Starting a TEMP instance */
int TEMP_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    char n[MOD_MAX_LENGTH_NAME];

    if (0 == ret)
    {
        if (_temp_nb_modules_started < _temp_nb_modules)
        {
            LOG_INF1("%s : starting instance number %d", TEMP_MODULE_NAME, _temp_nb_modules_started);
        }
        else
        {
            LOG_ERR ("%s : no available instance, %d > %d", TEMP_MODULE_NAME, _temp_nb_modules_started, _temp_nb_modules);
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Creating instance name */
        snprintf(n, MOD_MAX_LENGTH_NAME, "%s_tsk%d", TEMP_MODULE_NAME, _temp_nb_modules_started);

        /* Instance configuration */
        ret = MODULE_config(&temp_modules[_temp_nb_modules_started], n, m_main, m_mod);

        if (0 != ret)
            LOG_ERR("TEMP : could not configure module properly, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Thread creation */
        ret = OS_create_thread(&temp_modules[_temp_nb_modules_started].thread, (void *) &(temp_modules[_temp_nb_modules_started]));

        if (0 != ret)
            LOG_ERR("TEMP : could not create thread, ret = %d", ret);
    }

    if (0 == ret)
        /* Increasing number of instances started */
        _temp_nb_modules_started++;

    return ret;
}

int TEMP_stop(void)
{
    int ret = 0;

    LOG_INF1("TEMP : stopping module");

    if (_temp_nb_modules_started > 0)
    {
        /* Decreasing number of instances started */
        _temp_nb_modules_started--;

        /* Stopping execution */
        MODULE_exit( &(temp_modules[_temp_nb_modules_started]) );

        /* Reattaching thread to avoid zombies */
        ret = OS_joint_thread(&temp_modules[_temp_nb_modules_started].thread, NULL);
    }

    return ret;
}
