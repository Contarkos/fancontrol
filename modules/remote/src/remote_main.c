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
t_mod_context remote_modules[] =
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

        /* Init message ID */
        .init_msg = REMOTE_INIT,
    }
};

static const t_uint32 _remote_nb_modules = sizeof(remote_modules) / sizeof (remote_modules[0]);
static       t_uint32 _remote_nb_modules_started = 0;

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Starting a REMOTE instance */
int REMOTE_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;
    char n[MOD_MAX_LENGTH_NAME];

    if (0 == ret)
    {
       if (_remote_nb_modules_started < _remote_nb_modules)
            LOG_INF1("%s : starting instance number %d", REMOTE_MODULE_NAME, _remote_nb_modules_started);
       else
       {
          LOG_ERR("REMOTE : no instance available, %d > %d", _remote_nb_modules_started, _remote_nb_modules);
          ret = -1;
       }
    }

    if (0 == ret)
    {
        /* Creating instance name */
        snprintf(n, MOD_MAX_LENGTH_NAME, "%s_tsk%d", REMOTE_MODULE_NAME, _remote_nb_modules_started);

        /* Instance configuration */
        ret = MODULE_config(&remote_modules[_remote_nb_modules_started], n, m_main, m_mod);

        if (0 != ret)
            LOG_ERR("REMOTE : could not configure module properly, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Thread creation */
        ret = OS_create_thread(&remote_modules[_remote_nb_modules_started].thread, (void *) &(remote_modules[_remote_nb_modules_started]));

        if (0 != ret)
            LOG_ERR("REMOTE : could not create thread, ret = %d", ret);
    }

    if (0 == ret)
        /* Increasing number of instances started */
        _remote_nb_modules_started++;

    return ret;
}

int REMOTE_stop(void)
{
    int ret = 0;

    LOG_INF1("REMOTE : stopping module");

    if (_remote_nb_modules_started > 0)
    {
        /* Decreasing number of instances started */
        _remote_nb_modules_started--;

        /* Stopping execution */
        MODULE_exit( &(remote_modules[_remote_nb_modules_started]) );

        /* Reattaching thread to avoid zombies */
        ret = OS_joint_thread(&remote_modules[_remote_nb_modules_started].thread, NULL);
    }

    return ret;
}

