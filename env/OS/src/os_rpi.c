
// Global includes
#include <stdio.h>

// Local includes
#include "os.h"
#include "os_rpi.h"

// Initialisation de la zone mémoire GPIO
struct bcm2835_peripheral os_periph_gpio = {GPIO_BASE, 0, NULL, NULL};

// Init des variables d'environnement
os_ret_okko is_init_gpio = OS_RET_KO;

int os_init_gpio(void)
{
    int ret = 0;

    if (OS_RET_OK == is_init_gpio)
    {
        printf("OS : init GPIO déjà effectué\n");
        ret = 1;
    }
    else
    {
        // Mapping du fichier mémoire
        ret += os_map_peripheral(&os_periph_gpio);

        if (0 != ret)
        {
            printf("OS : Erreur à l'init des GPIO, code : %d\n", ret);
        }
        else
        {
            printf("OS : Init GPIO ok\n");
            is_init_gpio = OS_RET_OK;
        }
    }

    return ret;
}

int os_stop_gpio(void)
{
    int ret = 0;

    // Reset des GPIO
//    GPIO_CLR_REGISTER = 0xFFFFFFFF;

    // Demapping du gpio
    os_unmap_peripheral(&os_periph_gpio);

    return ret;
}

// Choix de la direction pour une GPIO
int OS_set_gpio(t_uint32 i_pin, t_uint32 i_inout)
{
    int ret = 0;

    if (i_pin <= GPIO_MAX_NB)
    {
        if (i_inout)
        {
            OUT_GPIO(i_pin);
        }
        else
        {
            INP_GPIO(i_pin);
        }
    }
    else
    {
        printf("OS : Erreur numéro de pin GPIO : %d\n", i_pin);
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
        ret = -1;
    }

    return ret;
}
