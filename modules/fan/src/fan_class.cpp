// Includes globaux
#include <stdio.h>
#include <unistd.h>
#include <mutex>

// Includes locaux
#include "base.h"
#include "os.h"
#include "module.h"
#include "fan.h"
#include "fan_class.h"

/* Définition des constructeurs */
FAN::FAN(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m) : MODULE(mod_name, m)
{
    ;
}

FAN::~FAN()
{
    ;
}

int FAN::start_module()
{
    int ret = 0;

    printf("[IS] FAN : Démarrage de la classe du module\n");

    // Démarrage du timer pour la boucle
    this->timer_fd += OS_create_timer(FAN_TIMER_USEC, &FAN::fan_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (0 == this->timer_fd)
    {
        printf("[ER] FAN : erreur création timer de boucle\n");
        ret = -1;
    }
    else
    {
        // Set de la diode en entrée pour lire la vitesse
        ret += OS_set_gpio(FAN_PIN_IN, OS_GPIO_FUNC_IN);

        // Reglage source Clock sur PLL D
        ret += OS_pwm_set_clock_source(OS_CLOCK_SRC_PLLC);

        // Set de la diode en sortie en PWM
        ret += OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_ALT5);

        // Mode MS pour le PWM
        ret += OS_pwm_set_mode(OS_PWM_MODE_MSMODE);

        // Cycle par défaut : 1024
        ret += OS_pwm_set_precision(FAN_DEFAULT_PREC);

        // Reglage de la fréquence PWM
        ret += OS_pwm_set_frequency(FAN_PWM_FREQ);

        // Reglage du duty cycle par defaut : 50%
        ret += OS_pwm_set_dutycycle(FAN_DEFAULT_CYCLE);

        // Activation...
        ret += OS_pwn_enable(OS_RET_OK);
    }

    return ret;
}

// Arret spécifique à ce module
int FAN::stop_module()
{
    int ret = 0;

    return ret;
}

int FAN::exec_loop()
{
    int ret = 0;
    static int n = 0;
    static int is_onoff = 0;
    const int max = 100;

#if 0
    // Allumage de la diode
    ret = OS_write_gpio(FAN_PIN_OUT, is_onoff);

    printf("FAN : écriture dans la pin %d = %d, result = %d\n", FAN_PIN_OUT, is_onoff, ret);

    // Sleep pour ne pas aller trop vite
    usleep(5000000);
#else
    if (is_onoff)
    {
        OS_pwm_set_dutycycle((float) 0);
    }
    else
    {
        OS_pwm_set_dutycycle((float) 100);
    }

    usleep(10000000);
#endif
    is_onoff = 1 - is_onoff;

    // Condition de sortie
    if (n > max)
    {
        printf("[IS] FAN : fin du module\n");
        this->set_running(false);
    }
    else
    {
        n++;
    }

    return ret;
}

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
