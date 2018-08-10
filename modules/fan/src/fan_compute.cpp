// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

void FAN::fan_timer_handler(size_t i_timer_id, void * i_data)
{
    FAN *p_this = reinterpret_cast<FAN *> (i_data);

    if (p_this && (p_this->timer_fd == i_timer_id))
    {
        p_this->fan_compute_duty();
    }
}

int FAN::fan_compute_duty(void)
{
    int ret = 0;
    float duty = 0;

    switch (current_mode)
    {
        case FAN_MODE_AUTO:
            {
                // Recupération de la température extérieure ?

                // Puis asservissement en température
                duty = 100.0F;
            }
            break;
        case FAN_MODE_TEMP:
            break;
            {
                // Calcul ecart de température
                duty = 50.0F;
            }
        case FAN_MODE_RPM:
            break;
            {
                if (this->consigne_speed > FAN_MAX_SPEED)
                {
                    printf("[WG] FAN : vitesse trop haute\n");
                    this->fan_setSpeed(FAN_MAX_SPEED);

                    ret = 2;
                }

                duty = ( ((float) this->consigne_speed) * OS_MAX_PERCENT_PWM ) / FAN_MAX_SPEED;
            }
        default:
            {
                printf("[ER] FAN : Mode de FAN invalide\n");
                ret = -1;
            }
    }

    // En cas d'erreur
    if (ret >= 0)
    {
        OS_pwm_set_dutycycle(duty);
    }

    return ret;
}
