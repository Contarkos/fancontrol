/* Includes globaux */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

/* Includes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com_msg.h"
#include "module.h"
#include "shmd.h"

#include "fan.h"
#include "fan_class.h"


/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

static float compute_duty_hysteresis    (int i_cons, float i_current);
static float compute_duty_differential  (float i_ref, float i_current);
static float compute_duty_linear        (float i_current);
static inline float compute_duty_speed  (t_uint32 i_speed);

/*********************************************************************/
/*                      Fonctions de classe                          */
/*********************************************************************/

int FAN::fan_compute_duty(void)
{
    static float d = 0;
    static float sign = 1;

    int ret = 0;
    float duty = 0;

    switch (current_mode)
    {
        case FAN_MODE_AUTO:
            {
                if (FAN_TEMP_INVALID == (t_int32) this->room_temp)
                    /* Calcul en mode lineaire par morceaux */
                    duty = compute_duty_linear(this->current_temp);
                else
                    /* Calcul en mode differentiel */
                    duty = compute_duty_differential(this->room_temp, this->current_temp);
            }
            break;
        case FAN_MODE_TEMP:
            {
                /* Calcul ecart de température */
                duty = compute_duty_hysteresis(this->consigne_temp, this->current_temp);
            }
            break;
        case FAN_MODE_RPM:
            {
                if (this->consigne_speed > FAN_MAX_SPEED)
                {
                    LOG_WNG("FAN : speed too high, reducing @ %d RPM", FAN_MAX_SPEED);
                    this->fan_setConsSpeed(FAN_MAX_SPEED);

                    ret = 2;
                }

                duty = compute_duty_speed(this->consigne_speed);
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
        /* Borne du dutycycle */
        duty = BASE_BORNE(duty, OS_MIN_PERCENT_PWM, OS_MAX_PERCENT_PWM);

        /* Log de debug */
        LOG_INF3("FAN : dutycycle courant = %f", duty);

        if (d >= 100.0F)
            sign = -1;
        else if (d <= 0.0F)
            sign = 1;

        d = d + sign * 2.5F;

        duty = d;
        LOG_INF3("FAN : dutycycle courant = %f", duty);

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

int FAN::fan_treat_irq(int i_fd)
{
    static t_uint32 cpt;
    int ret = 0;
    t_uint32 v, ss;
    unsigned long d;

    if (0 == i_fd)
    {
        LOG_ERR("FAN : pas de file descriptor valide pour l'IRQ");
        ret = 2;
    }

    if (0 == ret)
    {
        /* Lecture des données */
        ss = read(i_fd, &(d), sizeof(d));

        if (sizeof(d) > ss)
        {
            LOG_WNG("FAN : mauvaise taille de message pour fd %d, ss = %d", i_fd, ss);
            ret = 4;
        }
    }

    if (0 == ret)
    {
        /* Conversion en vitesse de rotation (RPM) */
        v = (t_uint32) ( (60 * FAN_SEC_TO_MSEC) / (float) (FAN_HITS_PER_CYCLE * d) );
        cpt++;

        if (cpt > 10U)
        {
            LOG_INF3("FAN : vitesse du fan = %d, d = %ld", v, d);
            cpt = 0;
        }
        this->fan_setCurSpeed(v);
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
/*                      Fonctions statiques                          */
/*********************************************************************/

/**
 * Compute the duty cycle based on the current temperature and the
 * required one with an hysteresis.
 */
static float compute_duty_hysteresis  (int i_cons, float i_current)
{
    static bool h = true;
    int t;
    float d = 0;

    /* Gestion de l'hysteresis */
    /* FIXME : utilisation de float pour comparer des valeurs < 0.3F !!! */
    if (h)
    {
        t = i_cons - 1;

        if ( fabs((float)t - i_current) < FAN_PWM_ECART )
            h = false;
    }
    else
    {
        t = i_cons + 1;

        if ( i_current > (float)t )
            h = true;
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

    /* Puis asservissement en température */
    d = (( i_current - i_ref ) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
    return d;
}

static float compute_duty_linear (float i_current)
{
    float d = 0;

    /* Fonction affine par morceau */
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

