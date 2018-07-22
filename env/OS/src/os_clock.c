// Includes globaux
#include <stdio.h>

// Includes locaux
#include "os.h"
#include "os_rpi.h"

// Initialisation de la zone mémoire CLOCK
struct bcm2835_peripheral os_periph_clock = {CLOCK_BASE, 0, NULL, NULL};

// Init des variables d'environnement
os_ret_okko is_init_clock = OS_RET_KO;

// Sélection de la source pour la clock
int OS_clock_set_source(os_clock_source i_source)
{
    int ret = 0;

    // Arret de la CLOCK le temps de changer les paramètres
    CLOCK_GP0_CTL_REGISTER |= CLOCK_PASSWD_MASK & ~(CLOCK_ENAB_MASK);

    // Attente de la descente du flag BUSY
    while ( CLOCK_GP0_CTL_REGISTER & CLOCK_BUSY_MASK ) {}

    // Set de la source
    CLOCK_GP0_CTL_REGISTER  = CLOCK_PASSWD_MASK | (CLOCK_GP0_CTL_REGISTER & ~(CLOCK_SRC_MASK));
    CLOCK_GP0_CTL_REGISTER |= CLOCK_PASSWD_MASK | (CLOCK_SRC_MASK & i_source);

    // Reactivation de la clock
    CLOCK_GP0_CTL_REGISTER |= CLOCK_PASSWD_MASK | CLOCK_ENAB_MASK;

    return ret;
}

int os_init_clock(void)
{
    int ret = 0;

    if (OS_RET_OK == is_init_clock)
    {
        printf("OS : init CLOCK déjà effectué\n");
        ret = 1;
    }
    else
    {
        // Mapping du fichier mémoire
        ret += os_map_peripheral(&os_periph_clock);

        if (0 != ret)
        {
            printf("OS : Erreur à l'init des CLOCK, code : %d\n", ret);
        }
        else
        {
            printf("OS : Init CLOCK ok\n");
            is_init_clock = OS_RET_OK;
        }
    }

    return ret;
}

int os_stop_clock(void)
{
    int ret = 0;

    // Demapping du CLOCK
    os_unmap_peripheral(&os_periph_clock);

    return ret;
}
