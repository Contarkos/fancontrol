// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module.h"
#include "temp.h"
#include "fan.h"
#include "fan_class.h"

int FAN::fan_treat_msg(t_com_msg i_msg, int i_size)
{
    int ret = 0;

    if (0 == i_size)
    {
        LOG_WNG("FAN : mauvaise taille de message ");
        ret = -1;
    }
    else
    {
        switch (i_msg.id)
        {
            case MAIN_SHUTDOWN:
                ret = this->stop_and_exit();
                break;
            case FAN_MODE:
                ret = this->fan_update_mode((t_fan_mode *) i_msg.data);
                break;
            case FAN_POWER:
                ret = this->fan_update_power((t_fan_power_mode *) i_msg.data);
                break;
            case TEMP_DATA:
                ret = this->fan_update_data((t_temp_data *) i_msg.data);
                break;
            default:
                LOG_ERR("FAN : mauvaise ID pour message, id = %d", i_msg.id);
                ret = 1;
        }
    }

    return ret;
}

int FAN::fan_update_data(t_temp_data *i_data)
{
    int ret = 0;

    // Mise à jour des données venant du module de température
    if (TEMP_VALIDITY_VALID == i_data->fan_temp_valid)
    {
        LOG_INF3("FAN : température ventilateur mise à jour, T = %f", i_data->fan_temp);
        this->current_temp = (t_uint32) i_data->fan_temp;
    }
    else
    {
        LOG_WNG("FAN : mauvaise température pour le ventilateur, temp = %f", i_data->fan_temp);
        ret += 1;
    }

    if (TEMP_VALIDITY_VALID == i_data->room_temp_valid)
    {
        LOG_INF3("FAN : température piece mise à jour, T = %f", i_data->room_temp);
        this->room_temp = (t_uint32) i_data->room_temp;
    }
    else
    {
        LOG_WNG("FAN : mauvaise température pour la piece, temp = %f", i_data->room_temp);
        ret += 2;
    }

    return ret;
}

int FAN::fan_update_mode(t_fan_mode *i_data)
{
    int ret = 0;

    if (NULL == i_data)
    {
        LOG_ERR("FAN : pas de données pour MaJ du mode");
        ret = -1;
    }
    else
    {
        switch (i_data->mode)
        {
            case FAN_MODE_AUTO:
            case FAN_MODE_TEMP:
            case FAN_MODE_RPM:
                this->current_mode = i_data->mode;
                break;
            default:
                LOG_ERR("FAN : mauvaise valeur pour MaJ mode, mode = %d", i_data->mode);
                ret = -2;
                break;
        }
    }

    return ret;
}

int FAN::fan_update_power(t_fan_power_mode *i_data)
{
    int ret = 0;

    if (NULL == i_data)
    {
        LOG_ERR("FAN : pas de données pour MaJ du power mode");
        ret = -1;
    }
    else
    {
        switch (i_data->power_mode)
        {
            case FAN_POWER_MODE_OFF:
            case FAN_POWER_MODE_ON:
                // Sauvegarde de l'état
                this->current_power_mode = i_data->power_mode;

                // Set de pin selon l'etat en cours
                ret = OS_write_gpio(FAN_PIN_OUT, this->current_power_mode);

                break;
            default:
                LOG_ERR("FAN : mauvaise valeur pour MaJ power mode, power mode = %d", i_data->power_mode);
                ret = -2;
                break;
        }
    }

    return ret;
}


