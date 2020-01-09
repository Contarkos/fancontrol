/* Global includes */
#include <stdio.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

/*********************************************************************/
/*                             Typedef                               */
/*********************************************************************/

typedef struct
{
    t_os_state status;
    t_uint32 freq;
    float duty;
    t_uint32 precision;
    t_os_clock_source source;
    os_mash_mode mash;
    os_pwm_mode mode;
} t_os_pwm_setup;

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

/* Initialisation de la zone mémoire PWM */
struct bcm2835_peripheral os_periph_pwm = {PWM_BASE, 0, NULL, NULL};

/* Init des variables d'environnement */
t_os_ret_okko is_init_pwm = OS_RET_KO;

/* Variables statiques */
static t_uint32 os_pwm_freq = 25000;
static float os_pwm_duty = 0.0F;
static t_uint32 os_pwm_prec = 255;
static t_os_clock_source os_pwm_source = OS_CLOCK_SRC_PLLC;

static t_os_pwm_setup os_pwm_setup_array[] =
{
    /* PWM0 */
    {
        .status = OS_STATE_OFF,
        .freq = 25000,
        .duty = 0.0F,
        .precision = 255,
        .source = OS_CLOCK_SRC_PLLC,
        .mash = 0,
        .mode = 0,
    }
};

/* Mutex pour les acces aux registres PWM */
OS_mutex_t os_pwm_mutex = OS_INIT_MUTEX;

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

/* Activation du PWM */
int OS_pwn_enable(t_os_state i_enable)
{
    int ret = 0;

    ret = OS_mutex_lock(&os_pwm_mutex);

    if (0 != ret)
    {
        LOG_WNG("OS : error while locking mutex for PWM enabling, ret = %d", ret);
    }
    else
    {
        switch (i_enable)
        {
            case OS_STATE_ON:
                /* Activation sans configuration du PWM1 */
                PWM_CTL_REGISTER |= PWM_CTL_PWEN1_MASK;
                break;
            case OS_STATE_OFF:
                /* Désactivation du PWM */
                PWM_CTL_REGISTER &= ~(PWM_CTL_PWEN1_MASK);
                break;
            default:
                LOG_ERR("OS : wrong state for PWM");
                ret = -1;
                break;
        }

        if (0 == ret)
        {
            /* Petit temps d'arret pour éviter un crash du module PWM */
            OS_usleep(10);
        }

        ret = OS_mutex_unlock(&os_pwm_mutex);
    }

    return ret;
}

/* Sélection de la source pour la clock */
int OS_pwm_set_clock_source(t_os_clock_source i_source)
{
    int ret = 0;

    /* Sauvegarde de la valeur */
    switch (i_source)
    {
        case OS_CLOCK_SRC_GND :
        case OS_CLOCK_SRC_OSC :
        case OS_CLOCK_SRC_TST1:
        case OS_CLOCK_SRC_TST2:
        case OS_CLOCK_SRC_PLLA:
        case OS_CLOCK_SRC_PLLC:
        case OS_CLOCK_SRC_PLLD:
        case OS_CLOCK_SRC_HDMI:
            os_pwm_source = i_source;
            break;
        default:
            LOG_ERR("OS : bad clock source for PWM, source = %d", i_source);
            ret = -1;
            break;
    }

    if (0 != ret)
    {
        LOG_WNG("OS : error while locking mutex for PWM clock source, ret = %d", ret);
    }
    else
    {
        os_enable_pwm(OS_STATE_OFF);

        /* Prise de mutex pour la clock */
        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM source, ret = %d", ret);
        }
        else
        {
            /* Set de la source */
            CLOCK_PWM_CTL_REGISTER  = (CLOCK_PASSWD_MASK | CLOCK_PWM_CTL_REGISTER) & ~(CLOCK_SRC_MASK);
            CLOCK_PWM_CTL_REGISTER |=  CLOCK_PASSWD_MASK | (CLOCK_SRC_MASK & os_pwm_source);

            /* Libération mutex pour la clock */
            ret = OS_mutex_unlock(&os_pwm_mutex);
        }

        os_enable_pwm(OS_STATE_ON);
    }

    return ret;
}

/* Reglage de la fréquence d'émission des données */
int OS_pwm_set_frequency(t_uint32 i_freq)
{
    int ret = 0;
    t_uint32 divi = 1, divr = 0, divf = 0, data;

    if (OS_RET_KO == is_init_clock)
    {
        LOG_ERR("OS : pas d'init de la CLOCK");
        ret = -1;
    }
    else
    {
        if (i_freq > os_clock_max_freq[os_pwm_source] || i_freq == 0)
        {
            LOG_ERR("OS : clock frequency too high, freq = %d", i_freq);
            ret = -2;
        }
        else
        {
            /* Sauvagarde de la valeur de la fréquence */
            os_pwm_freq = i_freq;

            /* Calcul du diviseur */
            divi = os_clock_max_freq[os_pwm_source] / (os_pwm_freq * os_pwm_prec);
            divr = os_clock_max_freq[os_pwm_source] % (os_pwm_freq * os_pwm_prec);
            divf = (t_uint32) ((float) (divr * CLOCK_MAX_DIVISOR) / (float) (os_pwm_freq * os_pwm_prec));

            LOG_INF3("OS : PWM divisor = %d", divi);

            if (divi > CLOCK_MAX_DIVISOR)
            {
                LOG_WNG("OS : la fréquence sera plus haute qu'attendue. Diviseur max atteint");
                ret = 1;

                /* Maximum possible pour le diviseur */
                divi = CLOCK_MAX_DIVISOR;
                divf = 0;
            }

            /* Mise en forme de la donnée pour le registre de clock */
            data = ( CLOCK_DIVI_MASK & (divi << CLOCK_DIVI_SHIFT) ) | ( CLOCK_DIVF_MASK & (divf << CLOCK_DIVF_SHIFT) );

            /* On coupe la clock */
            os_enable_pwm(OS_STATE_OFF);

            ret = OS_mutex_lock(&os_pwm_mutex);

            if (0 != ret)
            {
                LOG_WNG("OS : error while locking mutex for PWM frequency");
            }
            else
            {
                /* Set de la fréquence */
                CLOCK_PWM_DIV_REGISTER = CLOCK_PASSWD_MASK | data;

                ret = OS_mutex_unlock(&os_pwm_mutex);
            }

            /* On reactive */
            os_enable_pwm(OS_STATE_ON);
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
        LOG_ERR("OS : mauvais pourcentage PWM, %% = %f", i_duty);
        ret = -1;
    }
    else
    {
        /* Sauvegarde de la valeur */
        os_pwm_duty = i_duty;

        /* Lock mutex PWM */
        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM dutycycle, ret = %d", ret);
        }
        else
        {
            /* Ecriture de la valeur dans le registre */
            PWM_DAT1_REGISTER = ((t_uint32) ((os_pwm_duty / OS_MAX_PERCENT_PWM) * (float) os_pwm_prec) );

            LOG_INF3("OS : dutycycle value = %d", PWM_DAT1_REGISTER);

            /* Release mutex PWM */
            ret = OS_mutex_unlock(&os_pwm_mutex);
        }
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
        LOG_ERR("OS : Valeur pour la précision PWM erronée : prec = %d", i_prec);
        ret = -1;
    }
    else
    {
        /* Sauvegarde la valeur */
        os_pwm_prec = i_prec;

        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM precision, ret = %d", ret);
        }
        else
        {
            /* Configuration du registre RNG1 */
            PWM_RNG1_REGISTER = os_pwm_prec;

            /* Release mutex PWM */
            if (0 != OS_mutex_unlock(&os_pwm_mutex))
            {
                LOG_ERR("OS  : error while unlocking mutex");
                ret = -2;
            }
            /* Reconfiguration de la frequence de la clock */
            else if (0 != OS_pwm_set_frequency(os_pwm_freq))
            {
                LOG_ERR("OS : error while recalibrating PWM frequency");
                ret = -4;
            }
            /* Reconfiguration du dutycycle */
            else if (0 != OS_pwm_set_dutycycle(os_pwm_duty))
            {
                LOG_ERR("OS : error while recalibrating PWM dutycyle");
                ret = -8;
            }
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

    /* Lock mutex PWM */
    ret = OS_mutex_lock(&os_pwm_mutex);

    if (0 != ret)
    {
        LOG_WNG("OS : error while locking mutex for PWM mode, ret = %d", ret);
    }
    else
    {
        switch (i_mode)
        {
            case OS_PWM_MODE_MSMODE:
                PWM_CTL_REGISTER |= PWM_CTL_MSEN1_MASK;
                break;
            case OS_PWM_MODE_PWMMODE:
                PWM_CTL_REGISTER &= ~PWM_CTL_MSEN1_MASK;
                break;
            default:
                LOG_WNG("OS : wrong mode for PWM");
                ret = -1;
                break;
        }

        ret = OS_mutex_unlock(&os_pwm_mutex);
    }

    return ret;
}

/*
 * Reglage du filtre MASH pour la division de frequence
 *
 *
 */
int OS_pwm_set_mash(os_mash_mode i_filter)
{
    int ret = 0;

    switch (i_filter)
    {
        case OS_PWM_MASH_FILTER_0:
        case OS_PWM_MASH_FILTER_1:
        case OS_PWM_MASH_FILTER_2:
        case OS_PWM_MASH_FILTER_3:
            break;
        default:
            LOG_WNG("OS : wrong value for MASH filter, filter = %d", i_filter);
            ret = -1;
            break;
    }

    if (ret < 0)
    {
        /* Ne rien faire */
        ;
    }
    else
    {
        /* On coupe la clock */
        os_enable_pwm(OS_STATE_OFF);

        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM mash filter, ret = %d", ret);
        }
        else
        {
            /* Update du filtre MASH */
            CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_MASH_MASK);
            CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) | (i_filter << CLOCK_MASH_SHIFT);

            ret = OS_mutex_unlock(&os_pwm_mutex);

            /* On reactive */
            os_enable_pwm(OS_STATE_ON);

            /* On actualise la frequence */
            OS_pwm_set_frequency(os_pwm_freq);
        }

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
        LOG_WNG("OS : init PWM déjà effectué");
        ret = 1;
    }
    else
    {
        /* Mapping du fichier mémoire */
        ret += os_map_peripheral(&os_periph_pwm);

        if (0 != ret)
        {
            LOG_ERR("OS : Erreur à l'init des PWM, code : %d", ret);
        }
        else
        {
            LOG_INF1("OS : Init PWM ok");
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
        LOG_WNG("OS : init PWM non effectué");
        ret = 1;
    }
    else
    {
        /* Demapping du PWM */
        os_unmap_peripheral(&os_periph_pwm);
    }

    return ret;
}

int os_enable_pwm(t_os_state i_enable)
{
    int ret = 0;

    ret = OS_mutex_lock(&os_pwm_mutex);

    if (0 != ret)
    {
        LOG_WNG("OS : error while locking mutex for PWM enabling, ret = %d", ret);
    }
    else
    {
        if (OS_STATE_OFF == i_enable)
        {
            /* Arret de la CLOCK le temps de changer les paramètres */
            CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_ENAB_MASK);

            /* Attente de la descente du flag BUSY */
            while ( CLOCK_PWM_CTL_REGISTER & CLOCK_BUSY_MASK ) {}
        }
        else if (OS_STATE_ON == i_enable)
        {
            /* Reactivation de la clock */
            CLOCK_PWM_CTL_REGISTER |= CLOCK_PASSWD_MASK | CLOCK_ENAB_MASK;
        }
        else
        {
            LOG_WNG("OS : wrong value to enable/disable PWM, value = %d", i_enable);
            ret = -1;
        }

        ret = OS_mutex_unlock(&os_pwm_mutex);
    }

    return ret;
}
