/* Includes globaux */

/* Includes locaux */
#include "base.h"
#include "com_msg.h"
#include "integ_log.h"
#include "remote.h"
#include "remote_class.h"

int REMOTE::remote_send_status(void)
{
    int ret = 0;
    t_remote_status s;

    s.fan_rpm = 1000;
    s.temp_temp = 25;

    ret = COM_send_data(this->udp_fd, REMOTE_STATUS, &s, sizeof(s), 0);
    LOG_INF3("REMOTE : envoi statut systeme, RPM = %d, temp = %d°, ret = %d", s.fan_rpm, s.temp_temp, ret);

    if (ret < 0)
        LOG_ERR("REMOTE : erreur a l'envoi du statut du systeme, ret = %d", ret);

    return ret;
}
