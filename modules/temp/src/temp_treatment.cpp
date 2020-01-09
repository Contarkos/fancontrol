/* Includes globaux */
#include <stdio.h>
#include <unistd.h>

/* Includes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module.h"
#include "temp.h"
#include "temp_class.h"
#include "fan.h"

int TEMP::temp_treat_msg()
{
    int ret = 0, ss;
    t_com_msg m;

    ret = COM_receive_data(this->p_fd[TEMP_FD_SOCKET].fd, &m, &ss);

    if (0 > ret)
    {
        LOG_ERR("TEMP : erreur à la réception des données, ret = %d", ret);
        ret = 1;
    }
    else if (0 == ss)
    {
        LOG_WNG("TEMP : mauvaise taille de message, ss = %d", ss);
        ret = 2;
    }
    else
    {
        /* Tout va bien, on lance les traitements */
        switch (m.id)
        {
            case MAIN_SHUTDOWN:
                ret = this->stop_and_exit();
                break;
            case TEMP_TIMER:
                ret = this->temp_retrieve_data();
                break;
            default:
                LOG_ERR("TEMP : mauvaise ID pour message, id = %d", m.id);
                ret = 1;
        }
    }
    return ret;
}

int TEMP::temp_treat_com(void)
{
    int ret = 0;
    t_com_msg_struct m;

    if (0 == ret)
        ret = COM_msg_read(COM_ID_TEMP, &m);

    if (0 == ret)
    {
        switch (m.header.id)
        {
            case MAIN_START:
                break;
            case MAIN_SHUTDOWN:
                ret = this->stop_and_exit();
                break;
            case TEMP_TIMER:
                ret = this->temp_retrieve_data();
                break;
            default:
                LOG_ERR("TEMP : mauvaise ID pour message, id = %d", m.header.id);
                ret = 1;
        }
    }

    return ret;
}
