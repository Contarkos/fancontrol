// Inlcudes locaux
#include "fan.h"
#include "fan_class.h"

// Includes globaux
#include <stdio.h>

// Variables globales

// Fonctions

int FAN_start(void)
{
    int ret = 0;
    std::mutex m;

    FAN *myFan (new FAN("FAN", &m));

    myFan->init_and_wait();

    myFan->~FAN();
    return ret;
}
