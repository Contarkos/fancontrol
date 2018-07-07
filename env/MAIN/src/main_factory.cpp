// Includes globaux

// Includes locaux
#include "base.h"
#include "base_typ.h"
#include "main_factory.h"
#include "module.h"
#include "fan.h"
#include "cmd.h"

// Variables globales

mod_type t_start[NB_MODULE] = {
        {&FAN_start, &FAN_stop}
};

int main_start_factory()
{
    int ret, ret_temp, ii;

    ret = 0;

    // On démarre les modules.
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        // On bloque les mutex de tout le monde
        t_start[ii].mod_mutex.lock();

        ret_temp = t_start[ii].mod_start(&(t_start[ii].mod_mutex));

        if (0 != ret_temp)
        {
            ret = 1;
            printf("[MAIN] : Erreur pendant le lancement du module n°%d\n", ii);
        }
    }

    // Tous les threads sont lancés on peut démarrer
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_start[ii].mod_mutex.unlock();
    }

    // On va chercher des commandes rentrées par l'utilisateur
    CMD_read();

    return ret;
}

int main_stop_factory()
{
    int ret = 0, ii;

    printf("MAIN : extinction des modules\n");

    // On lance les demandes d'arrets
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_start[ii].mod_stop();
    }

    // On attend que tous les threads soient terminés
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_start[ii].mod_mutex.lock();
    }

    return ret;
}
