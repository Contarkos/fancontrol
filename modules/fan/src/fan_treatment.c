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
#include "temp.h"

#include "fan.h"
#include "fan_module.h"

/*********************************************************************/
/*                         Global variables                          */
/*********************************************************************/

fan_e_mode       fan_current_mode;
fan_e_power_mode fan_current_power_mode;

/*********************************************************************/
/*                        Module functions                           */
/*********************************************************************/

int fan_treat_com(void)
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
                ret = fan_modules[0].stop_and_exit(&fan_modules[0]);
                break;
            case FAN_MODE:
                ret = fan_update_mode((t_fan_mode *) m.body);
                break;
            case FAN_POWER:
                ret = fan_update_power((t_fan_power_mode *) m.body);
                break;
            case FAN_TIMER:
                ret = fan_compute_duty();
                break;
            case TEMP_DATA:
                ret = fan_update_data((t_temp_data *) m.body);
                break;
            default:
                LOG_ERR("FAN : wrong ID for message, id = %d", m.header.id);
                ret = 1;
                break;
        }
    }

    return ret;
}

int fan_update_data(t_temp_data *i_data)
{
    int ret = 0;

    /* Update data coming from temperature module */
    if (TEMP_VALIDITY_VALID == i_data->fan_temp_valid)
    {
        LOG_INF3("FAN : fan temperature updated, T = %f", i_data->fan_temp);
        fan_current_temp = i_data->fan_temp;
    }
    else
    {
        LOG_WNG("FAN : wrong temperature for fan, temp = %f", i_data->fan_temp);
        fan_current_temp = FAN_TEMP_INVALID;
        ret += 1;
    }

    if (TEMP_VALIDITY_VALID == i_data->room_temp_valid)
    {
        LOG_INF3("FAN : room temperature updated, T = %f", i_data->room_temp);
        fan_room_temp = i_data->room_temp;
    }
    else
    {
        LOG_WNG("FAN : wrong temperature for room, temp = %f", i_data->room_temp);
        fan_room_temp = FAN_TEMP_INVALID;
        ret += 2;
    }

    return ret;
}

int fan_update_mode(t_fan_mode *i_data)
{
    int ret = 0;

    if (NULL == i_data)
    {
        LOG_ERR("FAN : no data for updating mode");
        ret = -1;
    }
    else
    {
        switch (i_data->mode)
        {
            case FAN_MODE_AUTO:
            case FAN_MODE_TEMP:
            case FAN_MODE_RPM:
                fan_current_mode = i_data->mode;
                break;
            default:
                LOG_ERR("FAN : wrong value to update mode, mode = %d", i_data->mode);
                ret = -2;
                break;
        }
    }

    return ret;
}

int fan_update_power(t_fan_power_mode *i_data)
{
    int ret = 0;

    if (NULL == i_data)
    {
        LOG_ERR("FAN : No data to update power mode");
        ret = -1;
    }

    if (0 == ret)
        ret = fan_set_power(i_data->power_mode);

    return ret;
}

int fan_set_power(fan_e_power_mode i_mode)
{
    int ret = 0;

    switch (i_mode)
    {
        case FAN_POWER_MODE_OFF:
        case FAN_POWER_MODE_ON:
            /* Saving state */
            fan_current_power_mode = i_mode;

            /* Set GPIO based on current state */
            ret = OS_write_gpio(FAN_PIN_OUT, fan_current_power_mode);

            break;
        default:
            LOG_ERR("FAN : wrong value to update power mode, power mode = %d", i_mode);
            ret = -1;
            break;
    }

    return ret;
}

/* Update the PWM registers based on the setup */
int fan_set_pwm(void)
{
    int ret = 0;
    /* Init the parameters for the PWM hardware */

    if (0 == ret)
        /* Setting clock source on PLL C */
        ret = OS_pwm_set_clock_source(OS_CLOCK_SRC_PLLC);

    if (0 == ret)
        /* MS mode for PWM */
        ret = OS_pwm_set_mode(OS_PWM_MODE_MSMODE);

    if (0 == ret)
        /* default cycle width : 1024 */
        ret = OS_pwm_set_precision(FAN_DEFAULT_PREC);

    if (0 == ret)
        /* Setting MASH filter default value */
        ret = OS_pwm_set_mash(OS_PWM_MASH_FILTER_1);

    if (0 == ret)
        /* Setting default PWM frequency */
        ret = OS_pwm_set_frequency(FAN_PWM_FREQ);

    if (0 == ret)
        /* Default dutycycle : 50% */
        ret = OS_pwm_set_dutycycle(FAN_DEFAULT_CYCLE);

    return ret;
}

