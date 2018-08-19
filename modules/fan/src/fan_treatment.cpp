// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module.h"
#include "temp.h"
#include "fan.h"
#include "fan_class.h"

int FAN::fan_treat_msg(t_com_msg i_msg)
{
    int ret = 0;

    switch (i_msg.id)
    {
        case MAIN_SHUTDOWN:
            ret = this->stop_and_exit();
            break;
        case TEMP_DATA:
            ret = this->fan_update_data((t_temp_data *) i_msg.data);
        default:
            printf("[ER] FAN : mauvaise ID pour message, id = %d", i_msg.id);
            ret = 1;
    }

    return ret;
}

int FAN::fan_update_data(t_temp_data * i_data)
{
    int ret = 0;

    // Mise à jour des données venant du module de température
    if (TEMP_VALIDITY_VALID == i_data->fan_temp_valid)
    {
        this->current_temp = (t_uint32) i_data->fan_temp;
    }
    else
    {
        ret += 1;
    }

    if (TEMP_VALIDITY_VALID == i_data->room_temp_valid)
    {
        this->room_temp = (t_uint32) i_data->room_temp;
    }
    else
    {
        ret += 2;
    }

    return ret;
}
