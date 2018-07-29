// Includes globaux
#include <mutex>

// Includes locaux
#include "base.h"
#include "base_typ.h"
#include "os.h"
#include "main_factory.h"
#include "module.h"
#include "fan.h"
#include "temp.h"
#include "cmd.h"

// Variables globales
std::mutex t_mutex[NB_MODULE];

mod_type t_start[NB_MODULE] = {
        {&FAN_start, &FAN_stop},
        {&TEMP_start, &TEMP_stop}
};

int main_start_factory()
{
    int ret, ret_temp, ii;

    ret = 0;

    // Init des librairies
    ret_temp = OS_init();

    if (0 != ret_temp)
    {
        printf("[ER] MAIN : erreur à l'init de l'OS, code : %d\n", ret_temp);
        ret = 1;
    }
    else
    {
        // On démarre les modules.
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            // On bloque les mutex de tout le monde
            t_mutex[ii].lock();

            ret_temp = t_start[ii].mod_start(&(t_mutex[ii]));

            if (0 != ret_temp)
            {
                ret = 1;
                printf("[ER] MAIN : Erreur pendant le lancement du module n°%d\n", ii);
            }
        }

        // Tous les threads sont lancés on peut démarrer
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            t_mutex[ii].unlock();
        }

        // On va chercher des commandes rentrées par l'utilisateur
        CMD_read();

    }

    return ret;
}

int main_stop_factory()
{
    int ret = 0, ii;

    printf("[IS] MAIN : extinction des modules\n");

    // On lance les demandes d'arrets
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_start[ii].mod_stop();
    }

    // On attend que tous les threads soient terminés
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_mutex[ii].lock();
    }

    // Arret des éléments du système
    ret += OS_stop();

    return ret;
}
