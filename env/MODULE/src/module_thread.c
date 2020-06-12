/* Gloval includes */

/* Local includes */
#include "base.h"
#include "base_typ.h"
#include "integ_log.h"
#include "os.h"

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

