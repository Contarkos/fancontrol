
/* Includes globaux */

/* Includes locaux */
#include "os.h"
#include "integ_log.h"

#include "module_bis.h"

void* MODULE_init (void *p_context)
{
    int ret = 0;
    t_mod_context *p = NULL;

    if (NULL == p_context)
    {
        LOG_ERR("MODULE : no context provided, cannot init");
        ret = -1;
    }

    if (0 == ret)
    {
        p = (t_mod_context *) p_context;

        if (BASE_FALSE == p->is_init)
        {
            LOG_ERR("MODULE : context not initialized, cannot start");
            ret = -1;
        }
    }

    if (0 == ret)
        ret = p->wait_and_loop(p);

    return (void *) NULL;
}

void* MODULE_exit (void *p_context)
{
    int ret = 0;
    t_mod_context *p = NULL;

    if (NULL == p_context)
    {
        LOG_ERR("MODULE : no context provided, cannot exit");
        ret = -1;
    }

    if (0 == ret)
    {
        p = (t_mod_context *) p_context;

        if (BASE_FALSE == p->is_init)
        {
            LOG_ERR("MODULE : context not initialized, cannot stop");
            ret = -1;
        }
    }

    if (0 == ret)
        ret = p->stop_and_exit(p);

    return (void *) NULL;
}

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

    /* Add the entry point for the thread */
    if (0 == ret)
        p_context->thread.loop = &MODULE_init;

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
        /* Lock of MAIN until the end of the init */
        OS_mutex_lock(p_context->mutex_main);
        p_context->is_init = BASE_TRUE;
    }

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

    if (0 == ret)
    {
        /* Unlocking MAIN thread */
        OS_mutex_unlock(p_context->mutex_main);

        /* Locking current thread waiting for GO from MAIN */
        OS_mutex_lock(p_context->mutex_mod);

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

