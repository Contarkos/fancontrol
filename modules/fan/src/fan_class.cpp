// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "os.h"
#include "module.h"
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
    ret += OS_create_timer();

    if (0 != ret)
    {
        printf("[ER] FAN : erreur création timer de boucle\n");
    }
    else
    {
        // Reglage source Clock sur oscillateur (19.2Mhz)
        ret += OS_clock_set_source(OS_CLOCK_SRC_OSC);

        // Set de la diode en sortie en PWM
        ret += OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_ALT4);

        // Set de la diode en entrée pour lire la vitesse
        ret += OS_set_gpio(FAN_PIN_IN, OS_GPIO_FUNC_IN);

        // Mode MS pour le PWM
        ret += OS_pwm_set_mode(OS_PWM_MODE_MSMODE);

        // Cycle par défaut : 1000
        ret += OS_pwm_set_precision(FAN_DEFAULT_PREC);

        // Reglage du duty cycle par defaut : 50%
        ret += OS_pwm_set_dutycycle(FAN_DEFAULT_CYCLE);

        // Reglage de la fréquence PWM
        ret += OS_pwm_set_frequency(FAN_PWM_FREQ);
    }

    return ret;
}

int FAN::stop_module()
{
    int ret = 0;

    // Arret de la boucle
    this->set_running(false);

    return ret;
}

int FAN::exec_loop()
{
    int ret = 0;
    static int n = 0;
    static int is_onoff = 0;
    const int max = 100000;

#if 0
    // Allumage de la diode
    ret = OS_write_gpio(FAN_PIN_OUT, is_onoff);

    printf("FAN : écriture dans la pin %d = %d, result = %d\n", FAN_PIN_OUT, is_onoff, ret);

    // Sleep pour ne pas aller trop vite
    usleep(5000000);
#else
    usleep(20);
#endif
    is_onoff = 1 - is_onoff;

    // Condition de sortie
    if (n > max)
    {
        printf("FAN : fin du module\n");
        this->set_running(false);
    }
    else
    {
        n++;
    }

    return ret;
}
