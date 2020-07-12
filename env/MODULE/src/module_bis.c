
/* Includes globaux */

/* Includes locaux */
#include "os.h"
#include "integ_log.h"

#include "module_bis.h"

int MODULE_config (t_mod_context *p_context,
                   const char name[MOD_MAX_LENGTH_NAME],
                   OS_mutex_t *p_mutex_main, OS_mutex_t *p_mutex_mod)
{
    int ret = 0;

    if (NULL == p_context)
    {
        LOG_ERR("MODULE : wrong context for module configuration");
        ret = -1;
    }

    if (0 == ret)
    {
        if ( (p_mutex_main != NULL) && (p_mutex_mod != NULL) )
        {
            p_context->mutex_main = p_mutex_main;
            OS_mutex_init(p_context->mutex_main);

            p_context->mutex_mod  = p_mutex_mod;
            OS_mutex_init(p_context->mutex_mod);
        }
        else
        {
            LOG_ERR("MODULE : wrong mutexes for module configuration");
            ret = -1;
        }
    }

    /* Test complete configuration */
    if (0 == ret)
    {
        if (
                /* Entry point and exit call */
                (NULL == p_context->init_module)
                || (NULL == p_context->exit_module)
                /* Main loop and exit function */
                || (NULL == p_context->wait_and_loop)
                || (NULL == p_context->stop_and_exit)
                /* Module specific functions */
                || (NULL == p_context->start_module)
                || (NULL == p_context->stop_module)
                || (NULL == p_context->init_after_wait)
                || (NULL == p_context->exec_loop)
            )
        {
            LOG_ERR("MODULE : incomplete context, cannot start");
            ret = -1;
        }
    }

    /* Add the entry point for the thread */
    if (0 == ret)
    {
        if (NULL != p_context->init_module)
            p_context->thread.loop = p_context->init_module;
        else
            ret = -1;
    }

    if (0 == ret)
    {
        if (name != NULL)
            strncpy(p_context->name, name, strnlen(name, MOD_MAX_LENGTH_NAME));
        else
        {
            LOG_ERR("MODULE : wrong name for module configuration");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Lock of MAIN's mutex until the end of the init */
        ret = OS_mutex_lock(p_context->mutex_main);

        if (ret != 0)
           LOG_ERR("MODULE : cannot lock MAIN mutex to wait for init");
    }

    /* Everything went smoothly up to here, YEAY. */
    if (0 == ret)
        p_context->is_init = BASE_TRUE;

    return ret;
}

int MODULE_wait_and_loop (t_mod_context *p_context)
{
    int ret = 0;

    if (NULL == p_context)
    {
        LOG_ERR("MODULE : wrong context provided, not starting");
        ret = -1;
    }

    if (0 == ret)
    {
        if (p_context->is_init == BASE_FALSE)
        {
            LOG_ERR("MODULE : context not initialized, aborting");
            ret = -2;
        }
    }

    /* Specific start for the module */
    if (0 == ret)
    {
        ret = p_context->start_module();

        if (0 != ret)
            LOG_ERR("MODULE : wrong start for %s, aborting", p_context->name);
    }

    /* Unlocking MAIN thread */
    if (0 == ret)
    {
        ret = OS_mutex_unlock(p_context->mutex_main);

        if (0 != ret)
           /* TODO : abort completely since MAIN is fucked up now ? */
           LOG_ERR("MODULE : mutex from MAIN not initialised for %s", p_context->name);
    }

    /* Locking current thread waiting for GO from MAIN */
    if (0 == ret)
    {
        ret = OS_mutex_lock(p_context->mutex_mod);

        if (0 != ret)
           LOG_ERR("MODULE : mutex for module %s not initialised", p_context->name);
    }

    if (0 == ret)
    {
        /* Run the few init actions left */
        ret = p_context->init_after_wait();

        if (0 != ret)
            LOG_ERR("%s : error in init after go, aborting", p_context->name);
    }

    /* Launch thread main loop */
    if (0 == ret)
    {
        p_context->is_running = BASE_TRUE;

        while (BASE_TRUE == p_context->is_running)
        {
            if (p_context->exec_loop() < 0)
            {
                LOG_ERR("%s : loop error, stopping thread", p_context->name);
                p_context->is_running = BASE_FALSE;
            }
        }

        /* Specific stop for the module */
        ret = p_context->stop_module();

        if (0 != ret)
            LOG_ERR("%s : error in specific stop", p_context->name);
    }

    /* Stop the thread no matter what happened before */
    if ( p_context && (BASE_TRUE == p_context->is_init) )
    {
        ret = p_context->stop_and_exit(p_context);

        if (0 != ret)
            LOG_ERR("%s : error while stopping module", p_context->name);

        /* Unlocking mutex for MAIN to stop properly */
        OS_mutex_unlock(p_context->mutex_mod);
    }

    return ret;
}

int MODULE_stop_and_exit(t_mod_context *p_context)
{
    int ret = 0;

    if (NULL == p_context)
    {
        LOG_ERR("MODULE : wrong context provided, cannot stop properly");
        ret = -1;
    }

    if (0 == ret)
    {
        if (p_context->is_init == BASE_FALSE)
        {
            LOG_ERR("MODULE : context not initialized, aborting");
            ret = -2;
        }
    }

    /* Stopping loop */
    if (0 == ret)
        p_context->is_running = BASE_FALSE;

    return ret;
}

