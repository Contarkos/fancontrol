// Includes globaux

// Includes locaux
#include "main_factory.h"
#include "module.h"
#include "fan.h"

// Variables globales
MODULE *tab_module[NB_MODULE + 1];

int main_start_factory()
{
    int ret = 0;


    FAN_start();

    return ret;
}

int main_stop_factory()
{
    int ret = 0;

    return ret;
}
