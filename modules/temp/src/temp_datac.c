/* Global includes */
#include <stdio.h>
#include <math.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module_bis.h"
#include "shmd.h"

#include "temp.h"
#include "temp_module.h"

t_temp_value temp_fan = { 0 };
t_temp_value temp_room = { 0 };

t_uint32 temp_adc_gain = 1;

/*******************************************************************/
/*                                                                 */
/*  description : Retrieve data from ADC board                     */
/*                                                                 */
/*  @out        : ret = 0  if OK                                   */
/*                ret > 0  on error                                */
/*                                                                 */
/*******************************************************************/
int temp_retrieve_data(void)
{
    int ret = 0;
    t_uint32 d;
    float r = 1, c;

    /* Activate GPIO connected to thermistor */
    ret = OS_write_gpio(TEMP_PIN_OUT, 1);

    if (ret < 0)
    {
        LOG_ERR("TEMP : error on GPIO for temperature reading, ret = %d", ret);
    }
    else
    {
        /* Read data on AD7705 */
        d = COM_adc_read_result(OS_SPI_DEVICE_0, COM_ADC_PAIR_1);

        /* Désactivation de la pin connectée au thermistor */
        /* FIXME : delete the comment */
        //ret = OS_write_gpio(TEMP_PIN_OUT, 0);

        if ( (0 == d) || (d == COM_ADC_MAXVALUE) )
        {
            LOG_WNG("TEMP : invalid temperature data, value = %d", d);
            ret = 1;

            /* Temperature is considered invalid */
            temp_fan.temp_valid = BASE_FALSE;
        }
        else
        {
            /* Computing equivalent resistor value */
            c = (TEMP_VREF_ADC / (float) ((float) temp_adc_gain * TEMP_VDD_ADC)) * (float) ( d - (COM_ADC_MAXVALUE >> 1) );
            r = (float) TEMP_THERM_COMP * (c + (COM_ADC_MAXVALUE >> 2)) / ((COM_ADC_MAXVALUE >> 2) - c);
            LOG_INF3("TEMP : resistor value, R = %f, c = %f", r, c);

            /* Calcul de la température (formule de Steinhart-Hart) */
            temp_fan.temp = (TEMP_THERM_COEFF / ( (TEMP_THERM_COEFF/TEMP_THERM_DEF_TEMP) + (float) (log(r) - log(TEMP_THERM_COMP)) ))
                - TEMP_THERM_K_TEMP;
            /*temp_fan_temp = 1 / ( (1/TEMP_THERM_DEF_TEMP) + (float) (log( r / TEMP_THERM_COMP ) / TEMP_THERM_COEFF) ); */

            /* Temperature validity */
            temp_fan.temp_valid = BASE_TRUE;
        }

        /* Envoi de la donnée à FAN */
        ret = temp_send_data();
    }

    if (0 == ret)
    {
        shmd_tempdata_t *p_temp;
        ret = SHMD_getPtrTempData(&p_temp);

        if (0 == ret)
        {
            p_temp->temp_sys        = temp_fan.temp;
            p_temp->temp_sys_valid  = temp_fan.temp_valid;

            ret = SHMD_givePtrTempData();
        }
    }


    return ret;
}

/*******************************************************************/
/*                                                                 */
/*  description : Send temperature data to everyone                */
/*                                                                 */
/*  @out        : ret = 0  if everything is ok                     */
/*                ret > 0  on error                                */
/*                                                                 */
/*******************************************************************/
int temp_send_data(void)
{
    int ret = 0;
    t_temp_data d;

    /* On remplit la structure */
    d.fan_temp = temp_fan.temp;
    d.fan_temp_valid = temp_fan.temp_valid ? TEMP_VALIDITY_VALID : TEMP_VALIDITY_INVALID;
    d.room_temp = temp_room.temp;
    d.room_temp_valid = temp_room.temp_valid ? TEMP_VALIDITY_VALID : TEMP_VALIDITY_INVALID;

    /* On envoie le tout */
    LOG_INF3("TEMP : sending temperature data");
    ret = COM_msg_send(TEMP_DATA, &d, sizeof(d));

    if (ret < 0)
        LOG_ERR("TEMP : error while sending data, ret = %d", ret);

    return ret;
}

/*******************************************************************/
/*                                                                 */
/*  description : Callback to treat IRQ                            */
/*                                                                 */
/*  @out        : ret = 0  if everything is ok                     */
/*                ret > 0  on error                                */
/*                                                                 */
/*******************************************************************/
int temp_treat_irq()
{
    int ret= 0, ss;
    unsigned long d;

    /* Acknowledge the IRQ */
    ss = read(temp_poll_fd[TEMP_FD_IRQ].fd, &d, sizeof(unsigned long));

    if (ss < 0)
    {
        LOG_WNG("TEMP : error while reading fd %d, ss = %d, errno = %d", TEMP_FD_IRQ, ss, errno);
        ret = 4;
    }
    else
    {
        /* Send tic to system */
        ret = temp_tic(d);
    }

    return ret;
}

/*******************************************************************/
/*                                                                 */
/*  description : Callback to treat IRQ                            */
/*                                                                 */
/*  @out        : ret = 0  if everything is ok                     */
/*                ret > 0  on error                                */
/*                                                                 */
/*******************************************************************/
int temp_tic(unsigned long i_tic)
{
    int ret = 0;
    t_temp_tic t;

    /* Fill message */
    t.tic = i_tic;

    /* Send tic message */
    ret = COM_msg_send(TEMP_TIC, &t, sizeof(t));

    return ret;
}

