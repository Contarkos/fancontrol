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
#include "fan.h"
#include "fan_class.h"


/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

static float compute_duty_hysteresis    (int i_cons, int i_current);
static float compute_duty_differential  (int i_ref, int i_current);
static float compute_duty_linear        (int i_current);
static inline float compute_duty_speed  (t_uint32 i_speed);

/*********************************************************************/
/*                      Fonctions de classe                          */
/*********************************************************************/

void FAN::fan_timer_handler(int i_timer_id, void * i_data)
{
    FAN *p_this = reinterpret_cast<FAN *> (i_data);
    int dum = 0;

    if (p_this && (p_this->timer_fd == i_timer_id))
    {
        COM_send_data(p_this->timeout_fd, FAN_TIMER, &dum, sizeof(dum), 0);
    }
}

int FAN::fan_compute_duty(void)
{
    int ret = 0;
    float duty = 0;
    static float d = 0;

    switch (current_mode)
    {
        case FAN_MODE_AUTO:
            {
                if (FAN_TEMP_INVALID == this->room_temp)
                {
                    /* Calcul en mode lineaire par morceaux */
                    duty = compute_duty_linear(this->current_temp);
                }
                else
                {
                    /* Calcul en mode differentiel */
                    duty = compute_duty_differential(this->room_temp, this->current_temp);
                }
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
                LOG_ERR("FAN : Mode de FAN invalide");
                ret = -1;
            }
            break;
    }

    /* En cas d'erreur */
    if (ret >= 0)
    {
        /* Borne du dutycycle */
        duty = BASE_BORNE(duty, OS_MIN_PERCENT_PWM, OS_MAX_PERCENT_PWM);

        /* Log de debug */
        LOG_INF3("FAN : dutycycle courant = %f", duty);

        if (d < 100.0F)
            d += 10.0F;
        else
            d = 0;

        duty = d;

        OS_pwm_set_dutycycle(duty);
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
    else
    {
        /* Lecture des données */
        ss = read(i_fd, &(d), sizeof(d));

        if (sizeof(d) > ss)
        {
            LOG_WNG("FAN : mauvaise taille de message pour fd %d, ss = %d", i_fd, ss);
            ret = 4;
        }
        else
        {
            /* Conversion en vitesse de rotation (RPM) */
            v = (t_uint32) ( FAN_SEC_TO_MSEC / (float) (FAN_HITS_PER_CYCLE * d) );
            cpt++;

            if (cpt > 100U)
            {
                LOG_INF3("FAN : vitesse du fan = %d, d = %ld", v, d);
                cpt = 0;
            }
            this->fan_setCurSpeed(v);
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
static float compute_duty_hysteresis  (int i_cons, int i_current)
{
    static bool h = true;
    int t;
    float d = 0;

    /* Gestion de l'hysteresis */
    /* FIXME : utilisation de float pour comparer des valeurs < 0.3F !!! */
    if (h)
    {
        t = i_cons - 1;

        if ( fabs((float) (t) - (float) (i_current)) < FAN_PWM_ECART )
            h = false;
    }
    else
    {
        t = i_cons + 1;

        if ( i_current > t )
            h = true;
    }

    d = ((float) ( i_current - t ) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
    return d;
}

static float compute_duty_differential (int i_ref, int i_current)
{
    float d = 0;

    /* Puis asservissement en température */
    d = ((float) ( i_current - i_ref ) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
    return d;
}

static float compute_duty_linear (int i_current)
{
    static float t = 0;
    float d = 0;

    /* Fonction affine par morceau */
    if ((float) i_current > FAN_TEMP_MAX)
    {
        d = FAN_DUTY_MAX;
    }
    else if ((float) i_current > FAN_TEMP_VERY_HIGH)
    {
        d = ((FAN_DUTY_MAX - FAN_DUTY_VERY_HIGH) / (FAN_TEMP_MAX - FAN_TEMP_VERY_HIGH)) * (float) i_current + FAN_DUTY_VERY_HIGH;
    }
    else if ((float) i_current > FAN_TEMP_HIGH)
    {
        d = ((FAN_DUTY_VERY_HIGH - FAN_DUTY_HIGH) / (FAN_TEMP_VERY_HIGH - FAN_TEMP_HIGH)) * (float) i_current + FAN_DUTY_HIGH;
    }
    else if ((float) i_current > FAN_TEMP_MEDIUM)
    {
        d = ((FAN_DUTY_HIGH - FAN_DUTY_MEDIUM) / (FAN_TEMP_HIGH - FAN_TEMP_MEDIUM)) * (float) i_current + FAN_DUTY_MEDIUM;
    }
    else if ((float) i_current > FAN_TEMP_LOW)
    {
        d = ((FAN_DUTY_MEDIUM - FAN_DUTY_LOW) / (FAN_TEMP_MEDIUM - FAN_TEMP_MEDIUM)) * (float) i_current + FAN_DUTY_LOW;
    }
    else
    {
        d = ((FAN_DUTY_LOW - FAN_DUTY_MIN) / (FAN_TEMP_LOW - FAN_TEMP_MIN)) * (float) i_current + FAN_DUTY_MIN;
    }

    if (t > 50)
    {
        t = 0;
    }
    else
    {
        t += 5;
    }

    /* FIXME : debug value, to delete */
    d = 50 + t;
    d = 80;

    return d;
}

static inline float compute_duty_speed (t_uint32 i_speed)
{
    float d = ( ((float) i_speed) * OS_MAX_PERCENT_PWM ) / FAN_MAX_SPEED;

    return d;
}

