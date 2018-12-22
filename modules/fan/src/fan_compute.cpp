// Includes globaux
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com_msg.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

void FAN::fan_timer_handler_old(int i_timer_id, void * i_data)
{
    FAN *p_this = reinterpret_cast<FAN *> (i_data);

    if (p_this && (p_this->timer_fd == i_timer_id))
    {
        p_this->fan_compute_duty();
    }
}

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
    int ret = 0, t;
    float duty = 0;
    static bool hysteresis = true;

    switch (current_mode)
    {
        case FAN_MODE_AUTO:
            {
                // Selection de la température de référence
                t = room_temp;

                // Puis asservissement en température
                duty = ((float) ( current_temp - t ) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
            }
            break;
        case FAN_MODE_TEMP:
            break;
            {
                // Gestion de l'hysteresis
                if (hysteresis)
                {
                    t = consigne_temp - 1;

                    if ( abs(t - current_temp) < FAN_PWM_ECART )
                    {
                        hysteresis = false;
                    }
                }
                else
                {
                    t = consigne_temp + 1;

                    if ( current_temp > t )
                    {
                        hysteresis = true;
                    }
                }

                // Calcul ecart de température
                duty = ((float) ( current_temp - t ) * OS_MAX_PERCENT_PWM) / FAN_ECART_MAX_TEMP;
            }
        case FAN_MODE_RPM:
            break;
            {
                if (this->consigne_speed > FAN_MAX_SPEED)
                {
                    LOG_WNG("FAN : vitesse trop haute");
                    this->fan_setConsSpeed(FAN_MAX_SPEED);

                    ret = 2;
                }

                duty = ( ((float) this->consigne_speed) * OS_MAX_PERCENT_PWM ) / FAN_MAX_SPEED;
            }
        default:
            {
                LOG_ERR("FAN : Mode de FAN invalide");
                ret = -1;
            }
    }

    // En cas d'erreur
    if (ret >= 0)
    {
        // Borne du dutycycle
        duty = BASE_BORNE(duty, OS_MIN_PERCENT_PWM, OS_MAX_PERCENT_PWM);

        OS_pwm_set_dutycycle(duty);
    }

    return ret;
}

int FAN::fan_treat_irq(int i_fd)
{
    int v, ret = 0;
    t_uint32 ss;
    unsigned long d;

    if (0 == i_fd)
    {
        LOG_ERR("FAN : pas de file descriptor valide pour l'IRQ");
        ret = 2;
    }
    else
    {
        // Lecture des données
        ss = read(i_fd, &(d), sizeof(d));

        if (sizeof(d) > ss)
        {
            LOG_WNG("FAN : mauvaise taille de message pour fd %d, ss = %d", i_fd, ss);
            ret = 4;
        }
        else
        {
            // Conversion en vitesse de rotation (RPM)
            v = (int) ( FAN_SEC_TO_MSEC / (float) (FAN_HITS_PER_CYCLE * d) );

            LOG_INF3("FAN : vitesse du fan = %d, d = %ld", v, d);
            this->fan_setCurSpeed(v);
        }
    }

    return ret;
}
