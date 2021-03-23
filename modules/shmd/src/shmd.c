
/* Global includes */

/* Local includes */
#include "base_typ.h"
#include "com.h"
#include "com_msg.h"
#include "integ_log.h"

#include "shmd.h"
/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

/* Mutex propre au module */
OS_mutex_t *shmd_mutex_mod = NULL;

/* Donnees systemes */
shmd_tempdata_t shmd_g_tempdata;
OS_mutex_t shmd_g_tempdata_mutex = OS_INIT_MUTEX;

shmd_fanstatus_t shmd_g_fanstatus;
OS_mutex_t shmd_g_fanstatus_mutex = OS_INIT_MUTEX;

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

int SHMD_start(OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    int ret = 0;

    /* Locking MAIN until end of init */
    if (0 == ret)
    {
        ret = OS_mutex_lock(m_main);

        if (0 != ret)
            LOG_ERR("SHMD : could not lock MAIN mutex");
    }

    if (0 == ret)
    {
        if (NULL == m_mod)
        {
            LOG_ERR("SHMD : bad mutex pointer at start up");
            ret = -1;
        }
        else
            shmd_mutex_mod = m_mod;
    }

    /* Init of semaphores if needed */
    if (0 == ret)
    {
        ;
    }

    /* Sending init message for SHMD module */
    t_com_msg_init msg = { .status = 0, };
    if (0 == ret)
        msg.status = 1;
    else
        msg.status = -1;

    ret = COM_msg_send(SHMD_INIT, &msg, sizeof(msg));

    if (0 != ret)
        LOG_ERR("SHMD : could not send INIT message");

    /* Unlocking main */
    ret = OS_mutex_unlock(m_main);

    return ret;
}

int SHMD_stop(void)
{
    int ret = 0;

    /* Suppression des semaphores */
    LOG_INF1("SHMD : stopping module");

    /* Deblocage du module pour que MAIN s'arrete */
    if (NULL == shmd_mutex_mod)
    {
        LOG_ERR("SHMD : bad pointer to module mutex");
        ret = -1;
    }
    else
    {
        ret = OS_mutex_unlock(shmd_mutex_mod);

        if (ret != 0)
           LOG_ERR("SHMD : could not unlock mutex properly");
    }

    return ret;
}

/*********************************************************************/
/*                           Temp DATA                             */
/*********************************************************************/

int SHMD_getPtrTempData(shmd_tempdata_t **p_o_data)
{
    int ret = 0;

    ret = OS_mutex_lock(&shmd_g_tempdata_mutex);

    if (0 != ret)
    {
        LOG_ERR("SHMD : error while taking mutex lock for SysData, ret = %d", ret);
    }
    else
    {
        /* Recuperation du pointeur vers les donnees */
        *(p_o_data) = &shmd_g_tempdata;
    }

    return ret;
}

int SHMD_givePtrTempData()
{
    int ret = 0;

    ret = OS_mutex_unlock(&shmd_g_tempdata_mutex);

    return ret;
}

int SHMD_getPtrFanStatus(shmd_fanstatus_t **p_o_data)
{
    int ret = 0;

    ret = OS_mutex_lock(&shmd_g_fanstatus_mutex);

    if (0 != ret)
    {
        LOG_ERR("SHMD : error while taking mutex lock for SysData, ret = %d", ret);
    }
    else
    {
        /* Recuperation du pointeur vers les donnees */
        *(p_o_data) = &shmd_g_fanstatus;
    }

    return ret;
}

int SHMD_givePtrFanStatus(void)
{
    int ret = 0;

    ret = OS_mutex_unlock(&shmd_g_fanstatus_mutex);

    return ret;
}
