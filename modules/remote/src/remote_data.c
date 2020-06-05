/* Global includes */

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "com_msg.h"
#include "shmd.h"

#include "remote.h"
#include "remote_module.h"

int remote_send_status(void)
{
    int ret = 0;
    t_remote_status s;

    s.fan_rpm = 1000;

    if (0 == ret)
    {
        shmd_tempdata_t *p_temp;
        ret = SHMD_getPtrTempData(&p_temp);

        if (0 == ret)
        {
            s.temp_temp = p_temp->temp_sys;
            s.temp_valid = p_temp->temp_sys_valid;

            ret = SHMD_givePtrTempData();
        }
    }

    if (0 == ret)
    {
        shmd_fanstatus_t *p_fan;
        ret = SHMD_getPtrFanStatus(&p_fan);

        if (0 == ret)
        {
            s.fan_rpm = p_fan->fan_speed;

            ret = SHMD_givePtrFanStatus();
        }
    }

    if (0 == ret)
    {
        ret = COM_send_mcast_data(&remote_out_socket, REMOTE_STATUS, &s, sizeof(s), 0);
        LOG_INF3("REMOTE : sending system status, RPM = %d, temp = %fC, ret = %d", s.fan_rpm, s.temp_temp, ret);

        if (ret < 0)
            LOG_ERR("REMOTE : error system status, ret = %d", ret);
    }


    return ret;
}

