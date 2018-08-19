// Includes globaux
#include <stdio.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "os_rpi.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

// Initialisation de la zone mémoire CLOCK
struct bcm2835_peripheral os_periph_clock = {CLOCK_BASE, 0, NULL, NULL};

// Init des variables d'environnement
t_os_ret_okko is_init_clock = OS_RET_KO;

// Init source clock
t_os_clock_source os_clock_source = OS_CLOCK_SRC_PLLC;

// Tableau des fréquences max pour chaque source de clock
t_uint32 os_clock_max_freq[] =
{
    0,            // OS_CLOCK_SRC_GND 
    19200000,     // OS_CLOCK_SRC_OSC 
    0,            // OS_CLOCK_SRC_TST1
    0,            // OS_CLOCK_SRC_TST2
    0,            // OS_CLOCK_SRC_PLLA
    1000000000,   // OS_CLOCK_SRC_PLLC
    500000000,    // OS_CLOCK_SRC_PLLD
    216000000     // OS_CLOCK_SRC_HDMI
};

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Sélection de la source pour la clock
int OS_clock_set_source(t_os_clock_source i_source)
{
    int ret = 0;

    // Prise de mutex pour la clock

    // Sauvegarde de la valeur
    os_clock_source = i_source;

    // Arret de la CLOCK le temps de changer les paramètres
    CLOCK_GP0_CTL_REGISTER = (CLOCK_GP0_CTL_REGISTER | CLOCK_PASSWD_MASK) & ~(CLOCK_ENAB_MASK);

    // Attente de la descente du flag BUSY
    while ( CLOCK_GP0_CTL_REGISTER & CLOCK_BUSY_MASK ) {}

    // Set de la source
    CLOCK_GP0_CTL_REGISTER  = (CLOCK_PASSWD_MASK | CLOCK_GP0_CTL_REGISTER) & ~(CLOCK_SRC_MASK);
    CLOCK_GP0_CTL_REGISTER |=  CLOCK_PASSWD_MASK | (CLOCK_SRC_MASK & os_clock_source);

    // Reactivation de la clock
    CLOCK_GP0_CTL_REGISTER |=  CLOCK_PASSWD_MASK | CLOCK_ENAB_MASK;

    // Libération mutex pour la clock

    return ret;
}

// Choix de la fréquence de l'horloge
int OS_clock_set_freq(t_uint32 i_freq)
{
    int ret = 0;

    UNUSED_PARAMS(i_freq);

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

int os_init_clock(void)
{
    int ret = 0;

    if (OS_RET_OK == is_init_clock)
    {
        printf("[WG] OS : init CLOCK déjà effectué\n");
        ret = 1;
    }
    else
    {
        // Mapping du fichier mémoire
        ret += os_map_peripheral(&os_periph_clock);

        if (0 != ret)
        {
            printf("[ER] OS : Erreur à l'init des CLOCK, code : %d\n", ret);
        }
        else
        {
            printf("[IS] OS : Init CLOCK ok\n");
            is_init_clock = OS_RET_OK;
        }
    }

    return ret;
}

int os_stop_clock(void)
{
    int ret = 0;

    if (OS_RET_KO == is_init_clock)
    {
        printf("[WG] OS : init CLOCK non effectué\n");
        ret = 1;
    }
    else
    {
        // Demapping du CLOCK
        os_unmap_peripheral(&os_periph_clock);
    }

    // Sauvegarde STOP
    is_init_clock = OS_RET_KO;

    return ret;
}

