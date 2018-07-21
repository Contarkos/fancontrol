#include "os.h"


// Init de toutes les fonctions nécessaires au Rpi
int OS_init(void)
{
    int ret = 0;

    // Init des GPIO
    ret = OS_init_gpio();

    // Init COM

    return ret;
}

// Arret de toutes les fonctions du Rpi
int OS_stop(void)
{
    int ret = 0;

    // Init des GPIO
    ret = OS_stop_gpio();

    // Init COM

    return ret;
}
