/* Global includes */
#include <stdio.h>
#include <unistd.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "com.h"
#include "com_msg.h"
#include "module_bis.h"

#include "remote_module.h"

/*********************************************************************/
/*                       Module functions                            */
/*********************************************************************/

int remote_treat_com()
{
    int ret = 0;
    t_com_msg_struct m;

    if (0 == ret)
        ret = COM_msg_read(COM_ID_REMOTE, &m);

    if (0 == ret)
    {
        LOG_INF3("REMOTE : received a message, ID = %d", m.header.id);

        switch (m.header.id)
        {
            case MAIN_START:
                break;
            case MAIN_SHUTDOWN:
                ret = remote_modules[0].stop_and_exit(&remote_modules[0]);
                break;
            case REMOTE_TIMER:
                ret = remote_send_status();
                break;
            default:
                LOG_ERR("REMOTE : unknown message ID, id = %d", m.header.id);
                ret = 1;
        }
    }

    return ret;
}

/* Handling message coming from outside */
int remote_treat_udp(int i_fd)
{
    int ret = 0, ss;
    t_com_msg m;

    if (i_fd < 0)
    {
        LOG_ERR("REMOTE : wrong file descriptor for UDP socket, fd = %d", i_fd);
        ret = -1;
    }
    else
    {
        /* Get message data */
        ret = COM_receive_data(i_fd, &m, &ss);

        if (0 == ss)
        {
            LOG_WNG("REMOTE : wrong data size");
            ret = -2;
        }
        else
        {
            switch (m.id)
            {
                default:
                    break;
            }
        }
    }

    return ret;
}
