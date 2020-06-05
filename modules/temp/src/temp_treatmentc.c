/* Global includes */
#include <stdio.h>
#include <unistd.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module_bis.h"
#include "fan.h"

#include "temp.h"
#include "temp_module.h"

/*********************************************************************/
/*                             Functions                             */
/*********************************************************************/

int temp_treat_com(void)
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
                ret = temp_modules[0].stop_and_exit(&temp_modules[0]);
                break;
            case TEMP_TIMER:
                ret = temp_retrieve_data();
                break;
            case TEMP_TIC:
                break;
            default:
                LOG_ERR("TEMP : wrong ID for message, id = %d", m.header.id);
                ret = 1;
        }
    }

    return ret;
}
