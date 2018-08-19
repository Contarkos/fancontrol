// Global includes
#include <stdio.h>

// Local includes
#include "base.h"
#include "os.h"
#include "os_rpi.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

// Initialisation de la zone mémoire PWM
struct bcm2835_peripheral os_periph_pwm = {PWM_BASE, 0, NULL, NULL};

// Init des variables d'environnement
t_os_ret_okko is_init_pwm = OS_RET_KO;

// Variables statiques
static t_uint32 os_pwm_freq = 25000;
static float os_pwm_duty = 0.0F;
static t_uint32 os_pwm_prec = 255;
static t_os_clock_source os_pwm_source = OS_CLOCK_SRC_PLLC;

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Activation du PWM
int OS_pwn_enable(t_os_state i_enable)
{
    int ret = 0;

    switch (i_enable)
    {
        case OS_STATE_ON:
            // Activation sans configuration du PWM1
            PWM_CTL_REGISTER |= PWM_CTL_PWEN1_MASK;
            break;
        case OS_STATE_OFF:
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
        OS_usleep(10);
    }

    return ret;
}

// Sélection de la source pour la clock
int OS_pwm_set_clock_source(t_os_clock_source i_source)
{
    int ret = 0;

    // Prise de mutex pour la clock

    // Sauvegarde de la valeur
    os_pwm_source = i_source;

    // Arret de la CLOCK le temps de changer les paramètres
    CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_ENAB_MASK);

    // Attente de la descente du flag BUSY
    while ( CLOCK_PWM_CTL_REGISTER & CLOCK_BUSY_MASK ) {}

    // Set de la source
    CLOCK_PWM_CTL_REGISTER  = (CLOCK_PASSWD_MASK | CLOCK_PWM_CTL_REGISTER) & ~(CLOCK_SRC_MASK);
    CLOCK_PWM_CTL_REGISTER |=  CLOCK_PASSWD_MASK | (CLOCK_SRC_MASK & os_clock_source);

    // Reactivation de la clock
    CLOCK_PWM_CTL_REGISTER |=  CLOCK_PASSWD_MASK | CLOCK_ENAB_MASK;

    // Libération mutex pour la clock

    return ret;
}

// Reglage de la fréquence d'émission des données
int OS_pwm_set_frequency(t_uint32 i_freq)
{
    int ret = 0;
    t_uint32 divi = 1, divr = 0, divf = 0, data;

    if (OS_RET_KO == is_init_clock)
    {
        printf("[ER] OS : pas d'init de la CLOCK\n");
        ret = -1;
    }
    else
    {
        if (i_freq > os_clock_max_freq[os_clock_source] || i_freq == 0)
        {
            printf("[ER] OS : Fréquence d'horloge trop haute, freq = %d\n", i_freq);
            ret = -2;
        }
        else
        {
            // Sauvagarde de la valeur de la fréquence
            os_pwm_freq = i_freq;

            // Calcul du diviseur
            divi = os_clock_max_freq[os_clock_source] / (os_pwm_freq * os_pwm_prec);
            divr = os_clock_max_freq[os_clock_source] % (os_pwm_freq * os_pwm_prec);
            divf = (t_uint32) ((float) (divr * CLOCK_MAX_DIVISOR) / (float) (os_pwm_freq * os_pwm_prec));

            printf("[IS] OS : diviseur pour PWM = %d\n", divi);

            if (divi > CLOCK_MAX_DIVISOR)
            {
                printf("[WG] OS : la fréquence sera plus haute qu'attendue. Diviseur max atteint\n");
                ret = 1;

                // Maximum possible pour le diviseur
                divi = CLOCK_MAX_DIVISOR;
            }

            // Mise en forme de la donnée pour le registre de clock
            data = ( CLOCK_DIVI_MASK & (divi << CLOCK_DIVI_SHIFT) ) | ( CLOCK_DIVF_MASK & (divf << CLOCK_DIVF_SHIFT) );

            // Arret de la CLOCK le temps de changer les paramètres
            CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_ENAB_MASK);

            // Attente de la descente du flag BUSY
            while ( CLOCK_PWM_CTL_REGISTER & CLOCK_BUSY_MASK ) {}

            // Set de la fréquence
            CLOCK_PWM_DIV_REGISTER = CLOCK_PASSWD_MASK | data;

            // Reactivation de la clock
            CLOCK_PWM_CTL_REGISTER |= CLOCK_PASSWD_MASK | CLOCK_ENAB_MASK;
        }
    }

    return ret;
}

/*
 * Reglage de la valeur du pourcentage de temps pendant lequel le signal est haut.
 *
 * Ce réglage dépend de la fréquence des données et doit être mis à jour quand cette
 * dernière est modifiée
 */
int OS_pwm_set_dutycycle(float i_duty)
{
    int ret = 0;

    if (i_duty < OS_MIN_PERCENT_PWM || i_duty > OS_MAX_PERCENT_PWM)
    {
        printf("[ER] OS : mauvais pourcentage PWM, %% = %f\n", i_duty);
        ret = -1;
    }
    else
    {
        // Sauvegarde de la valeur
        os_pwm_duty = i_duty;

        // Ecriture de la valeur dans le registre
        PWM_DAT1_REGISTER = ((t_uint32) ((os_pwm_duty / OS_MAX_PERCENT_PWM) * (float) os_pwm_prec) );

        printf("[IS] OS : dutycycle value = %d\n", PWM_DAT1_REGISTER);
    }

    return ret;
}

/*
 * Réglage de la précision (range) utilisée pour envoyer les données.
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

        // Reconfiguration de la frequence de la clock
        ret = OS_pwm_set_frequency(os_pwm_freq);

        if (0 != ret)
        {
            printf("[ER] OS : Erreur recalibration de la fréquence\n");
        }
        else
        {
            // Reconfiguration du dutycycle
            ret = OS_pwm_set_dutycycle(os_pwm_duty);
        }
    }

    return ret;
}

/*
 * Régalge du mode de répartition des 0 et des 1 sur la période :
 *
 * MS mode : S (range) bits envoyés/période, M (data) bits à 1, le reste à 0
 *
 * PWM mode : les bits à 1 (data) sont répartis équitablement sur les bits de
 * période (range)
 */
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

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

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

    if (OS_RET_KO == is_init_pwm)
    {
        printf("[WG] OS : init PWM non effectué\n");
        ret = 1;
    }
    else
    {
        // Demapping du PWM
        os_unmap_peripheral(&os_periph_pwm);
    }

    return ret;
}
