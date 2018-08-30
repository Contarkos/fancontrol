// Includes globaux
#include <mutex>

// Includes locaux
#include "base.h"
#include "base_typ.h"
#include "integ_log.h"
#include "os.h"
#include "main_factory.h"
#include "module.h"
#include "fan.h"
#include "temp.h"
#include "cmd.h"
#include "com.h"

// Variables globales
std::mutex t_mod_mutex[NB_MODULE];
std::mutex t_main_mutex[NB_MODULE];
int main_is_running = 0;

mod_type t_start[NB_MODULE] = {
        {&FAN_start, &FAN_stop},
        {&TEMP_start, &TEMP_stop}
};

int main_start_factory()
{
    int ret, ret_temp, ii;

    ret = 0;

    // Init des librairies
    ret_temp = main_init();

    if (0 != ret_temp)
    {
        ret = 1;
    }
    else
    {
        // On démarre les modules.
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            // On bloque les mutex de tout le monde
            LOG_INF1("MAIN : lock pour %d", ii);
            t_mod_mutex[ii].lock();
            t_main_mutex[ii].unlock();

            ret_temp = t_start[ii].mod_start(&(t_main_mutex[ii]), &(t_mod_mutex[ii]));

            if (0 != ret_temp)
            {
                ret = 1;
                LOG_ERR("MAIN : Erreur pendant le lancement du module n°%d", ii);
            }
        }

        // Attente du retour des threads du module
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            LOG_INF1("MAIN : lock d'attente pour %d", ii);
            t_main_mutex[ii].lock();
        }

        // Tous les threads sont lancés on peut démarrer
        for (ii = 0; ii < NB_MODULE; ii++)
        {
            LOG_INF1("MAIN : unlock pour %d", ii);
            t_mod_mutex[ii].unlock();
        }

        // C'est parti
        main_is_running = 1;

        // On va chercher des commandes rentrées par l'utilisateur
        CMD_read();

    }

    return ret;
}

int main_init(void)
{
    int ret = 0;

    // Init de l'OS
    ret = OS_init();

    // Init de COM
    if (ret != 0)
    {
        LOG_ERR("MAIN : erreur à l'init de l'OS, code : %d", ret);
    }
    else
    {
        ret = COM_init();
    }

    return ret;
}

int main_stop_factory()
{
    int ret = 0, ii;

    LOG_INF1("MAIN : extinction des modules");

    // On lance les demandes d'arrets
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_start[ii].mod_stop();
    }

    LOG_INF1("MAIN : Attente des locks à libérer pour les modules");

    // On attend que tous les threads soient terminés
    for (ii = 0; ii < NB_MODULE; ii++)
    {
        t_mod_mutex[ii].lock();
    }

    // Arret des éléments du système
    LOG_INF1("MAIN : Arret des modules systemes");
    ret += main_stop();

    return ret;
}

int main_stop(void)
{
    int ret = 0;

    // Arret de la COM exterieure
    ret += COM_stop();

    // Arret de l'OS
    ret += OS_stop();

    return ret;
}

