// Global includes
#include <stdio.h>
#include <time.h>

// Local includes
#include "os.h"
#include "os_rpi.h"

#define MIN_PERCENT_PWM     0.0F
#define MAX_PERCENT_PWM     100.0F

// Initialisation de la zone mémoire PWM
struct bcm2835_peripheral os_periph_pwm = {PWM_BASE, 0, NULL, NULL};

// Init des variables d'environnement
os_ret_okko is_init_pwm = OS_RET_KO;

// Variables statiques
static t_uint32 os_pwm_freq = 25000;
static float os_pwm_duty = 0.0F;
static t_uint32 os_pwm_prec = 255;

int OS_pwn_enable(os_ret_okko i_enable)
{
    int ret = 0;

    switch (i_enable)
    {
        case OS_RET_OK:
            // Activation sans configuration du PWM1
            PWM_CTL_REGISTER |= PWM_CTL_PWEN1_MASK;
            break;
        case OS_RET_KO:
            // Désactivation du PWM
            PWM_CTL_REGISTER &= PWM_CTL_PWEN1_MASK;
            break;
        default:
            printf("[ER] OS : wrong state for PWM\n");
            ret = -1;
            break;
    }

    if (0 == ret)
    {
        // Petit temps d'arret pour éviter un crash du module PWM
        struct timespec timer_enbl = {.tv_sec = 0, .tv_nsec = 10000};

        nanosleep(&timer_enbl, NULL);
    }

    return ret;
}

// Reglage de la fréquence d'émission des données
int OS_pwm_set_frequency(t_uint32 i_freq)
{
    int ret = 0;
    t_uint32 divisor = 1;

    if (OS_RET_KO == is_init_clock)
    {
        printf("[ER] OS : pas d'init de la CLOCK\n");
        ret = -1;
    }
    else
    {
        if (i_freq > CLOCK_MAX_FREQ)
        {
            printf("[ER] OS : Fréquence d'horloge trop haute, freq = %d\n", i_freq);
            ret = -2;
        }
        else
        {
            // Sauvagarde de la valeur de la fréquence
            os_pwm_freq = i_freq;

            // Calcul du diviseur
            divisor = CLOCK_MAX_FREQ / (os_pwm_freq * os_pwm_prec);

            if (divisor > CLOCK_MAX_DIVISOR)
            {
               divisor = CLOCK_MAX_DIVISOR;
            }

            // Arret de la CLOCK le temps de changer les paramètres
            CLOCK_GP0_CTL_REGISTER = (CLOCK_GP0_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_ENAB_MASK);

            // Attente de la descente du flag BUSY
            while ( CLOCK_GP0_CTL_REGISTER & CLOCK_BUSY_MASK ) {}

            // Set de la fréquence
            CLOCK_GP0_DIV_REGISTER = CLOCK_PASSWD_MASK | ( CLOCK_DIVI_MASK & (divisor << CLOCK_DIVI_SHIFT) );

            // Reactivation de la clock
            CLOCK_GP0_CTL_REGISTER |= CLOCK_PASSWD_MASK | CLOCK_ENAB_MASK;
        }
    }

    return ret;
}

// Reglage de la valeur du pourcentage de temps où le signal est haut.
int OS_pwm_set_dutycycle(float i_duty)
{
    int ret = 0;

    if (i_duty < MIN_PERCENT_PWM || i_duty > MAX_PERCENT_PWM)
    {
        printf("[ER] OS : mauvais pourcentage PWM, %% = %f\n", i_duty);
        ret = -1;
    }
    else
    {
        // Sauvegarde de la valeur
        os_pwm_duty = i_duty;

        // Ecriture de la valeur dans le registre
        PWM_DAT1_REGISTER = ((t_uint32) ((os_pwm_duty / MAX_PERCENT_PWM) * (float) os_pwm_prec) );
    }

    return ret;
}

/*
 * Réglage de la précision utilisée pour envoyer les données.
 *
 * Correspond au nombre total de bits envoyés par période (les 0 et les 1)
 */
int OS_pwm_set_precision(t_uint32 i_prec)
{
    int ret = 0;

    if ( i_prec > MAX_UINT_16 )
    {
        printf("[ER] OS | Valeur pour la précision PWM erronée : prec = %d\n", i_prec);
        ret = -1;
    }
    else
    {
        // Sauvegarde la valeur
        os_pwm_prec = i_prec;

        // Configuration du registre RNG1
        PWM_RNG1_REGISTER = os_pwm_prec;
    }

    return ret;
}

int OS_pwm_set_mode(os_pwm_mode i_mode)
{
    int ret = 0;

    switch (i_mode)
    {
        case OS_PWM_MODE_MSMODE:
            PWM_CTL_REGISTER |= PWM_CTL_MSEN1_MASK;
            break;
        case OS_PWM_MODE_PWMMODE:
            PWM_CTL_REGISTER &= ~PWM_CTL_MSEN1_MASK;
            break;
        default:
            printf("[WG] OS : wrong mode for PWM\n");
            ret = -1;
            break;
    }

    return ret;
}

int os_init_pwm(void)
{
    int ret = 0;

    if (OS_RET_OK == is_init_pwm)
    {
        printf("[WG] OS : init PWM déjà effectué\n");
        ret = 1;
    }
    else
    {
        // Mapping du fichier mémoire
        ret += os_map_peripheral(&os_periph_pwm);

        if (0 != ret)
        {
            printf("[ER] OS : Erreur à l'init des PWM, code : %d\n", ret);
        }
        else
        {
            printf("[IS] OS : Init PWM ok\n");
            is_init_pwm = OS_RET_OK;
        }
    }

    return ret;
}

int os_stop_pwm(void)
{
    int ret = 0;

    // Demapping du PWM
    os_unmap_peripheral(&os_periph_pwm);

    return ret;
}
