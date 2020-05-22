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
#include "fan.h"
#include "fan_class.h"

int FAN::fan_treat_msg(int i_fd)
{
    int ret = 0, ss;
    t_com_msg m;

    if (0 > i_fd)
    {
        LOG_ERR("FAN : pas de file descriptor pour lecture message, fd = %d", i_fd);
        ret = -1;
    }
    else
    {
        /* Recuperation des données du message */
        ret = COM_receive_data(i_fd, &m, &ss);

        if (0 == ss)
        {
            LOG_WNG("FAN : mauvaise taille de message ");
            ret = -2;
        }
        else
        {
            switch (m.id)
            {
                case MAIN_SHUTDOWN:
                    ret = this->stop_and_exit();
                    break;
                case FAN_MODE:
                    ret = this->fan_update_mode((t_fan_mode *) m.data);
                    break;
                case FAN_POWER:
                    ret = this->fan_update_power((t_fan_power_mode *) m.data);
                    break;
                case FAN_TIMER:
                    ret = this->fan_compute_duty();
                    break;
                case TEMP_DATA:
                    ret = this->fan_update_data((t_temp_data *) m.data);
                    break;
                default:
                    LOG_ERR("FAN : mauvaise ID pour message, id = %d", m.id);
                    ret = 1;
            }
        }
    }

    return ret;
}

int FAN::fan_treat_com(void)
{
    int ret = 0;
    t_com_msg_struct m;

    if (0 == ret)
        ret = COM_msg_read(COM_ID_FAN, &m);

    if (0 == ret)
    {
        LOG_INF3("FAN : received a message, ID = %d", m.header.id);

        switch (m.header.id)
        {
            case MAIN_START:
                break;
            case MAIN_SHUTDOWN:
                ret = this->stop_and_exit();
                break;
            case FAN_MODE:
                ret = this->fan_update_mode((t_fan_mode *) m.body);
                break;
            case FAN_POWER:
                ret = this->fan_update_power((t_fan_power_mode *) m.body);
                break;
            case FAN_TIMER:
                LOG_INF3("FAN : just received FAN_TIMER");
                ret = this->fan_compute_duty();
                break;
            case TEMP_DATA:
                ret = this->fan_update_data((t_temp_data *) m.body);
                break;
            default:
                LOG_ERR("FAN : wrong ID for message, id = %d", m.header.id);
                ret = 1;
                break;
        }
    }

    return ret;
}

int FAN::fan_update_data(t_temp_data *i_data)
{
    int ret = 0;

    /* Mise à jour des données venant du module de température */
    if (TEMP_VALIDITY_VALID == i_data->fan_temp_valid)
    {
        LOG_INF3("FAN : fan temperature updated, T = %f", i_data->fan_temp);
        this->current_temp = i_data->fan_temp;
    }
    else
    {
        LOG_WNG("FAN : wrong temperature for fan, temp = %f", i_data->fan_temp);
        this->current_temp = FAN_TEMP_INVALID;
        ret += 1;
    }

    if (TEMP_VALIDITY_VALID == i_data->room_temp_valid)
    {
        LOG_INF3("FAN : room temperature updated, T = %f", i_data->room_temp);
        this->room_temp = i_data->room_temp;
    }
    else
    {
        LOG_WNG("FAN : wrong temperature for room, temp = %f", i_data->room_temp);
        this->room_temp = FAN_TEMP_INVALID;
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

    if (0 == ret)
        ret = this->fan_set_power(i_data->power_mode);

    return ret;
}

int FAN::fan_set_power(fan_e_power_mode i_mode)
{
    int ret = 0;

    switch (i_mode)
    {
        case FAN_POWER_MODE_OFF:
        case FAN_POWER_MODE_ON:
            /* Sauvegarde de l'état */
            this->current_power_mode = i_mode;

            /* Set de pin selon l'etat en cours */
            ret = OS_write_gpio(FAN_PIN_OUT, this->current_power_mode);

            break;
        default:
            LOG_ERR("FAN : mauvaise valeur pour MaJ power mode, power mode = %d", i_mode);
            ret = -1;
            break;
    }

    return ret;
}

/* Update the PWM registers based on the setup */
int FAN::fan_set_pwm(void)
{
    int ret = 0;
    /* Init the parameters for the PWM hardware */

    if (0 == ret)
        /* Reglage source Clock sur PLL C */
        ret = OS_pwm_set_clock_source(OS_CLOCK_SRC_PLLC);

    if (0 == ret)
        /* Mode MS pour le PWM */
        ret = OS_pwm_set_mode(OS_PWM_MODE_MSMODE);

    if (0 == ret)
        /* Cycle par défaut : 1024 */
        ret = OS_pwm_set_precision(FAN_DEFAULT_PREC);

    if (0 == ret)
        /* Reglage du filter MASH par défaut */
        ret = OS_pwm_set_mash(OS_PWM_MASH_FILTER_1);

    if (0 == ret)
        /* Reglage de la fréquence PWM */
        ret = OS_pwm_set_frequency(FAN_PWM_FREQ);

    if (0 == ret)
        /* Reglage du duty cycle par defaut : 50% */
        ret = OS_pwm_set_dutycycle(FAN_DEFAULT_CYCLE);

    return ret;
}
