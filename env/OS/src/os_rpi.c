
// Global includes
#include <stdio.h>
#include <linux/gpio.h>

// Local includes
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

// Initialisation de la zone mémoire GPIO
struct bcm2835_peripheral os_periph_gpio = {GPIO_BASE, 0, NULL, NULL};

// Init des variables d'environnement
t_os_ret_okko is_init_gpio = OS_RET_KO;

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Choix de la direction pour une GPIO
int OS_set_gpio(t_uint32 i_pin, t_os_gpio_func i_inout)
{
    int ret = 0;

    if (i_pin <= GPIO_MAX_NB)
    {
        switch (i_inout)
        {
            case OS_GPIO_FUNC_IN:
                {
                    INP_GPIO(i_pin);
                }
                break;
            case OS_GPIO_FUNC_OUT:
                {
                    OUT_GPIO(i_pin);
                }
                break;
            case OS_GPIO_FUNC_ALT0:
            case OS_GPIO_FUNC_ALT1:
            case OS_GPIO_FUNC_ALT2:
            case OS_GPIO_FUNC_ALT3:
            case OS_GPIO_FUNC_ALT4:
            case OS_GPIO_FUNC_ALT5:
                {
                    SET_GPIO_ALT(i_pin, i_inout);
                }
                break;
            default:
                LOG_ERR("OS : mauvaise fonction pour GPIO, func = %d", i_inout);
                ret = -2;
                break;
        }

    }
    else
    {
        LOG_ERR("OS : Erreur numéro de pin, GPIO = %d", i_pin);
        ret = -1;
    }

    return ret;
}

// Ecriture dans une GPIO avec wiringPi
int OS_write_gpio(t_uint32 i_pin, t_uint32 bool_active)
{
    int ret = 0;

    if (i_pin <= GPIO_MAX_NB)
    {
        if (bool_active)
        {
            GPIO_SET(i_pin);
        }
        else
        {
            GPIO_CLR(i_pin);
        }
    }
    else
    {
        LOG_ERR("OS : mauvaise valeur de pin GPIO, pin = %d", i_pin);
        ret = -1;
    }

    return ret;
}

// Lecture de la valeur d'une pin
int OS_read_gpio(t_uint32 i_pin)
{
    int data = 0;

    if (i_pin <= GPIO_MAX_NB)
    {
        data = GPIO_READ(i_pin);
    }
    else
    {
        LOG_ERR("OS : mauvaise valeur de pin GPIO, pin = %d", i_pin);
        data = -1;
    }

    return data;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

int os_init_gpio(void)
{
    int ret = 0;

    if (OS_RET_OK == is_init_gpio)
    {
        LOG_WNG("OS : init GPIO déjà effectué");
        ret = 1;
    }
    else
    {
        // Mapping du fichier mémoire
        ret += os_map_peripheral(&os_periph_gpio);

        if (0 != ret)
        {
            LOG_ERR("OS : Erreur à l'init des GPIO, code : %d", ret);
        }
        else
        {
            LOG_INF1("OS : Init GPIO ok");
            is_init_gpio = OS_RET_OK;
        }
    }

    return ret;
}

int os_stop_gpio(void)
{
    int ret = 0;

    // Demapping du gpio
    os_unmap_peripheral(&os_periph_gpio);

    return ret;
}

