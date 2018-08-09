// Ajout du support des handlers

// Global includes
#include <stdio.h>
#include <signal.h>

// Local includes
#include "base_typ.h"
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
            printf("[ER] MAIN : Erreur à l'arrêt des modules sur SIGINT\n");
        }
        else
        {
            // Arret de MAIN
            main_is_running = 0;
        }
    }
    else
    {
        ;
    }

    exit(ret);
}

int main_add_handlers(void)
{
    int ret = 0;

    // Ajout de tous les handlers pour les signaux simples
    printf("[IS] MAIN : Ajout des handlers simples\n");

    // Gestion du <C-c>
    signal(SIGINT, &main_sigint_handler);

    return ret;
}
