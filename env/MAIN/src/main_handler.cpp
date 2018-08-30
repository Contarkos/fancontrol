// Ajout du support des handlers

// Global includes
#include <stdio.h>
#include <signal.h>

// Local includes
#include "base_typ.h"
#include "integ_log.h"
#include "main.h"
#include "main_handler.h"
#include "main_factory.h"


void main_sigint_handler(int signum)
{
    int ret = 0;

    if (SIGINT == signum)
    {
        ret = main_stop_factory();

        if (0 != ret)
        {
            LOG_ERR("MAIN : Erreur à l'arrêt des modules sur SIGINT");
        }
        else
        {
            // Arret de MAIN
            main_is_running = 0;
        }

        exit(ret);
    }
    else
    {
        ;
    }
}

void main_sigkill_handler(int signum)
{
    int ret = 0;
    LOG_INF1("MAIN : Arrêt des modules sur SIGKILL");

    if (SIGKILL == signum)
    {
        ret = main_stop_factory();

        if (0 != ret)
        {
            LOG_ERR("MAIN : Erreur à l'arrêt des modules sur SIGKILL");
        }
        else
        {
            // Arret de MAIN
            main_is_running = 0;
        }

        exit(ret);
    }
    else
    {
        ;
    }
}

int main_add_handlers(void)
{
    int ret = 0;

    // Ajout de tous les handlers pour les signaux simples
    LOG_INF1("MAIN : Ajout des handlers simples");

    // Gestion du <C-c>
    signal(SIGINT, &main_sigint_handler);

    // Gestion du kill
    signal(SIGKILL, &main_sigkill_handler);

    // Ignore du broken pipe
    signal(SIGPIPE, SIG_IGN);

    return ret;
}

