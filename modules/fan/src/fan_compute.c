/* Global includes */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

/* Local includes*/
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com_msg.h"
#include "module_bis.h"
#include "shmd.h"

#include "fan.h"
#include "fan_module.h"

/*********************************************************************/
/*                         Global variables                          */
/*********************************************************************/

t_uint32 fan_consigne_speed;        /* Consigne de vitesse */
int      fan_consigne_temp;         /* Température consigne à atteindre */
float    fan_current_temp;          /* Température de l'élément à refroidir */
float    fan_room_temp;             /* Température de la pièce */

t_uint32 fan_current_speed;         /* Vitesse du ventilateur */

/*********************************************************************/
/*                       Static declarations                         */
/*********************************************************************/

static float compute_duty_hysteresis    (int i_cons, float i_current);
static float compute_duty_differential  (float i_ref, float i_current);
static float compute_duty_linear        (float i_current);
static inline float compute_duty_speed  (t_uint32 i_speed);

/*********************************************************************/
/*                        Module functions                           */
/*********************************************************************/

int fan_compute_duty(void)
{
    int ret = 0;
    float duty = 0;

    switch (fan_current_mode)
    {
        case FAN_MODE_AUTO:
            {
                if (FAN_TEMP_INVALID == (t_int32) fan_room_temp)
                    /* Piecewise affine mode */
                    duty = compute_duty_linear(fan_current_temp);
                else
                    /* Differential mode */
                    duty = compute_duty_differential(fan_room_temp, fan_current_temp);
            }
            break;
        case FAN_MODE_TEMP:
            {
                /* Computing based on temperature */
                duty = compute_duty_hysteresis(fan_consigne_temp, fan_current_temp);
            }
            break;
        case FAN_MODE_RPM:
            {
                if (fan_consigne_speed > FAN_MAX_SPEED)
                {
                    LOG_WNG("FAN : speed too high, reducing @ %d RPM", FAN_MAX_SPEED);
                    fan_consigne_speed = FAN_MAX_SPEED;

                    ret = 2;
                }

                duty = compute_duty_speed(fan_consigne_speed);
            }
            break;
        default:
            {
                LOG_ERR("FAN : invalid FAN mode");
                ret = -1;
            }
            break;
    }

    if (ret >= 0)
    {
        /* Boundaries of dutycycle */
        duty = BASE_BORNE(duty, OS_MIN_PERCENT_PWM, OS_MAX_PERCENT_PWM);

        /* Debug log */
        LOG_INF3("FAN : current dutycycle = %.2f", duty);

#if 0
        static float d = 0;
        static float sign = 1;

        if (d >= 100.0F)
            sign = -1;
        else if (d <= 0.0F)
            sign = 1;

        d = d + sign * 2.5F;

        duty = d;
        LOG_INF3("FAN : current dutycycle = %f", duty);
#endif
        ret = OS_pwm_set_dutycycle(duty);
    }

    /* Save the shared data */
    if (0 == ret)
    {
        shmd_fanstatus_t *p_fan;
        ret = SHMD_getPtrFanStatus(&p_fan);

        if (0 == ret)
        {
            p_fan->fan_target = (t_uint32) duty;

            ret = SHMD_givePtrFanStatus();
        }
    }

    return ret;
}

int fan_treat_irq(int i_fd)
{
    static t_uint32 cpt;
    int ret = 0;
    t_uint32 v;
    ssize_t ss;
    unsigned long d;

    if (0 == i_fd)
    {
        LOG_ERR("FAN : no valid file descriptor for the IRQ");
        ret = 2;
    }

    if (0 == ret)
    {
        /* Reading data from the IRS */
        ss = read(i_fd, &(d), sizeof(d));

        if (ss < 0)
        {
            LOG_ERR("FAN : error reading IRQ data, errno = %d", ss);
            ret = 1;
        }
        else if ((t_uint32)ss < sizeof(d))
        {
            LOG_WNG("FAN : wrong message size for fd %d, ss = %d", i_fd, ss);
            ret = 4;
        }
    }

    if (0 == ret)
    {
        /* Converting into rotation speed (RPM) */
        v = (t_uint32) ( (60 * FAN_SEC_TO_MSEC) / (float) (FAN_HITS_PER_CYCLE * d) );
        cpt++;

        if (cpt > 10U)
        {
            LOG_INF3("FAN : fan speed = %d, d = %ld", v, d);
            cpt = 0;
        }

        fan_current_speed = v;
    }

    /* Save the shared data */
    if (0 == ret)
    {
        shmd_fanstatus_t *p_fan;
        ret = SHMD_getPtrFanStatus(&p_fan);

        if (0 == ret)
        {
            p_fan->fan_speed = v;

            ret = SHMD_givePtrFanStatus();
        }
    }

    return ret;
}

/*********************************************************************/
/*                        Static functions                           */
/*********************************************************************/

/**
 * Compute the duty cycle based on the current temperature and the
 * required one with an hysteresis.
 */
static float compute_duty_hysteresis  (int i_cons, float i_current)
{
    static base_bool h = BASE_TRUE;
    int t;
    float d = 0;

    /* Hysteresis handling */
    /* FIXME : use float to compare values < 0.3F !!! */
    if (h)
    {
        t = i_cons - 1;

        if ( fabs((float)t - i_current) < FAN_PWM_ECART )
            h = BASE_FALSE;
    }
    else
    {
        t = i_cons + 1;

        if ( i_current > (float)t )
            h = BASE_TRUE;
    }

    if (i_current > (float)t)
        d = ((i_current - (float)t) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
    else
        d = 0;

    return d;
}

static float compute_duty_differential (float i_ref, float i_current)
{
    float d = 0;

    /* Temperature based dutycycle */
    d = (( i_current - i_ref ) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
    LOG_INF1("FAN : fan %.2fC, room %.2fC, speed = %.2f%%", i_current, i_ref, d);

    return d;
}

static float compute_duty_linear (float i_current)
{
    float d = 0;

    /* Affine function */
    if (i_current > FAN_TEMP_MAX)
    {
        d = FAN_DUTY_MAX;
    }
    else if (i_current > FAN_TEMP_VERY_HIGH)
    {
        d = ((FAN_DUTY_MAX - FAN_DUTY_VERY_HIGH) / (FAN_TEMP_MAX - FAN_TEMP_VERY_HIGH)) * i_current + FAN_DUTY_VERY_HIGH;
    }
    else if (i_current > FAN_TEMP_HIGH)
    {
        d = ((FAN_DUTY_VERY_HIGH - FAN_DUTY_HIGH) / (FAN_TEMP_VERY_HIGH - FAN_TEMP_HIGH)) * i_current + FAN_DUTY_HIGH;
    }
    else if (i_current > FAN_TEMP_MEDIUM)
    {
        d = ((FAN_DUTY_HIGH - FAN_DUTY_MEDIUM) / (FAN_TEMP_HIGH - FAN_TEMP_MEDIUM)) * i_current + FAN_DUTY_MEDIUM;
    }
    else if (i_current > FAN_TEMP_LOW)
    {
        d = ((FAN_DUTY_MEDIUM - FAN_DUTY_LOW) / (FAN_TEMP_MEDIUM - FAN_TEMP_MEDIUM)) * i_current + FAN_DUTY_LOW;
    }
    else
    {
        d = ((FAN_DUTY_LOW - FAN_DUTY_MIN) / (FAN_TEMP_LOW - FAN_TEMP_MIN)) * i_current + FAN_DUTY_MIN;
    }

    return d;
}

static inline float compute_duty_speed (t_uint32 i_speed)
{
    float d = ( ((float) i_speed) * OS_MAX_PERCENT_PWM ) / FAN_MAX_SPEED;

    return d;
}

