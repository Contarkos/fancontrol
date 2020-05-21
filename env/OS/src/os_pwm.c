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
/*                        Static functions                           */
/*********************************************************************/
static t_os_pwm_setup* os_pwm_get_device(t_os_pwm_device i_device);

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/

/* PWM mapping structure initialisation */
struct bcm2835_peripheral os_periph_pwm = {PWM_BASE, 0, NULL, NULL};

/* Environment variables initialisation */
t_os_ret_okko is_init_pwm = OS_RET_KO;

/* Mutex for PWM registers */
OS_mutex_t os_pwm_mutex = OS_INIT_MUTEX;

/*********************************************************************/
/*                        Static variables                           */
/*********************************************************************/
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
    },
    /* PWM1 */
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

/*********************************************************************/
/*                         API Functions                             */
/*********************************************************************/

/* PWM activation */
int OS_pwn_enable(t_os_state i_enable)
{
    int ret = 0;
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM enable");
        ret = -1;
    }

    if (0 == ret)
    {

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
                    /* Activation without configuration of PWM0 */
                    PWM_CTL_REGISTER |= PWM_CTL_PWEN1_MASK;
                    break;
                case OS_STATE_OFF:
                    /* Deactivation of PWM0 */
                    PWM_CTL_REGISTER &= ~(PWM_CTL_PWEN1_MASK);
                    break;
                default:
                    LOG_ERR("OS : wrong state for PWM");
                    ret = -1;
                    break;
            }

            if (0 == ret)
            {
                /* Small stop to avoid a crash of PWM module */
                OS_usleep(10);
            }

            ret = OS_mutex_unlock(&os_pwm_mutex);
        }
    }

    if (0 == ret)
        p_setup->status = i_enable;

    return ret;
}

/* Source selection for the clock */
int OS_pwm_set_clock_source(t_os_clock_source i_source)
{
    int ret = 0;
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM source");
        ret = -1;
    }

    if (0 == ret)
    {
        /* Save the value */
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
                p_setup->source = i_source;
                break;
            default:
                LOG_ERR("OS : bad clock source for PWM, source = %d", i_source);
                ret = -1;
                break;
        }

        if (0 != ret)
            LOG_WNG("OS : error while locking mutex for PWM clock source, ret = %d", ret);
    }

    if (0 == ret)
    {
        os_enable_pwm(OS_STATE_OFF);

        /* Lock mutex for the clock */
        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM source, ret = %d", ret);
        }
        else
        {
            /* Set source registers */
            CLOCK_PWM_CTL_REGISTER  = (CLOCK_PASSWD_MASK | CLOCK_PWM_CTL_REGISTER) & ~(CLOCK_SRC_MASK);
            CLOCK_PWM_CTL_REGISTER |=  CLOCK_PASSWD_MASK | (CLOCK_SRC_MASK & p_setup->source);

            /* Freeing mutex for the clock */
            ret = OS_mutex_unlock(&os_pwm_mutex);
        }

        os_enable_pwm(OS_STATE_ON);
    }

    return ret;
}

/* Setting the frequency for emitting new data */
int OS_pwm_set_frequency(t_uint32 i_freq)
{
    int ret = 0;
    t_uint32 divi = 1, divr = 0, divf = 0, data;
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM frequency");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_KO == is_init_clock)
        {
            LOG_ERR("OS : pas d'init de la CLOCK");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        if (i_freq > os_clock_max_freq[p_setup->source] || i_freq == 0)
        {
            LOG_ERR("OS : clock frequency too high, freq = %d", i_freq);
            ret = -2;
        }
    }

    if (0 == ret)
    {
        /* Save the new frequency */
        p_setup->freq = i_freq;

        /* Computing the divisor */
        divi = os_clock_max_freq[p_setup->source] / (p_setup->freq * p_setup->precision);
        divr = os_clock_max_freq[p_setup->source] % (p_setup->freq * p_setup->precision);
        divf = (t_uint32) ((float) (divr * CLOCK_MAX_DIVISOR) / (float) (p_setup->freq * p_setup->precision));

        LOG_INF3("OS : PWM divisor = %d", divi);

        if (divi > CLOCK_MAX_DIVISOR)
        {
            LOG_WNG("OS : frequency will be higher than expected. Max divisor reached");
            ret = 1;

            /* Maximum possible value for the divisor */
            divi = CLOCK_MAX_DIVISOR;
            divf = 0;
        }

        /* Computing data for clock register */
        data = ( CLOCK_DIVI_MASK & (divi << CLOCK_DIVI_SHIFT) ) | ( CLOCK_DIVF_MASK & (divf << CLOCK_DIVF_SHIFT) );

        /* Shutting down clock */
        os_enable_pwm(OS_STATE_OFF);

        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM frequency");
        }
        else
        {
            /* Set frequency register */
            CLOCK_PWM_DIV_REGISTER = CLOCK_PASSWD_MASK | data;

            ret = OS_mutex_unlock(&os_pwm_mutex);
        }

        /* Reenabling clock */
        os_enable_pwm(OS_STATE_ON);
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
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM dutycycle");
        ret = -1;
    }

    if (0 == ret)
    {
        if (i_duty < OS_MIN_PERCENT_PWM || i_duty > OS_MAX_PERCENT_PWM)
        {
            LOG_ERR("OS : mauvais pourcentage PWM, %% = %f", i_duty);
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Sauvegarde de la valeur */
        p_setup->duty = i_duty;

        /* Lock mutex PWM */
        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM dutycycle, ret = %d", ret);
        }
        else
        {
            /* Ecriture de la valeur dans le registre */
            PWM_DAT1_REGISTER = ((t_uint32) ((p_setup->duty / OS_MAX_PERCENT_PWM) * (float) p_setup->precision) );

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
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM precision");
        ret = -1;
    }

    if (0 == ret)
    {
        if ( i_prec > MAX_UINT_16 )
        {
            LOG_ERR("OS : Valeur pour la précision PWM erronée : prec = %d", i_prec);
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Sauvegarde la valeur */
        p_setup->precision = i_prec;

        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM precision, ret = %d", ret);
        }
        else
        {
            /* Configuration du registre RNG1 */
            PWM_RNG1_REGISTER = p_setup->precision;

            /* Release mutex PWM */
            if (0 != OS_mutex_unlock(&os_pwm_mutex))
            {
                LOG_ERR("OS  : error while unlocking mutex");
                ret = -2;
            }
        }
    }

    if (0 == ret)
    {
        ret = OS_pwm_set_frequency(p_setup->freq);

        /* Reconfiguration de la frequence de la clock */
        if (0 != ret)
        {
            LOG_ERR("OS : error while recalibrating PWM frequency");
            ret = -4;
        }
    }

    if (0 == ret)
    {
        ret = OS_pwm_set_dutycycle(p_setup->duty);

        /* Reconfiguration du dutycycle */
        if (0 != ret)
        {
            LOG_ERR("OS : error while recalibrating PWM dutycyle");
            ret = -8;
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
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM mode");
        ret = -1;
    }

    if (0 == ret)
    {
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
    }

    if (0 == ret)
        p_setup->mode = i_mode;

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
    t_os_pwm_setup *p_setup = os_pwm_get_device(OS_PWM_DEVICE_0);

    if (NULL == p_setup)
    {
        LOG_ERR("OS : wrong device for PWM mash");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_filter)
        {
            case OS_PWM_MASH_FILTER_0:
            case OS_PWM_MASH_FILTER_1:
            case OS_PWM_MASH_FILTER_2:
            case OS_PWM_MASH_FILTER_3:
                /* Save the current value */
                p_setup->mash = i_filter;
                break;
            default:
                LOG_WNG("OS : wrong value for MASH filter, filter = %d", i_filter);
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        /* Shutting down the clock */
        os_enable_pwm(OS_STATE_OFF);

        ret = OS_mutex_lock(&os_pwm_mutex);

        if (0 != ret)
        {
            LOG_WNG("OS : error while locking mutex for PWM mash filter, ret = %d", ret);
        }
        else
        {
            /* Update MASH filter */
            CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_MASH_MASK);
            CLOCK_PWM_CTL_REGISTER = (CLOCK_PWM_CTL_REGISTER | CLOCK_PASSWD_MASK) | (i_filter << CLOCK_MASH_SHIFT);

            ret = OS_mutex_unlock(&os_pwm_mutex);

            /* Enabling again */
            os_enable_pwm(OS_STATE_ON);
        }
    }

    if (0 == ret)
        /* On actualise la frequence */
        ret = OS_pwm_set_frequency(p_setup->freq);

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

static t_os_pwm_setup* os_pwm_get_device(t_os_pwm_device i_device)
{
    if (i_device < OS_PWM_DEVICE_NB)
        return &os_pwm_setup_array[i_device];
    else
        return NULL;
}

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
