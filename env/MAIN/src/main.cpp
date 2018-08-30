// Include globaux
#include <stdio.h>
#include <unistd.h>
#include <mutex>

// Include de base
#include "base.h"
#include "base_typ.h"
#include "integ_log.h"

// Include locaux
#include "main_handler.h"
#include "main_factory.h"

static int main_config(int argc, char *argv[])
{
    int ii;
    char *a;

    if (argc > 0)
    {
        LOG_INF1("MAIN : There is %d args which are : ", argc);

        for (ii = 0, a = argv[0]; ii < argc; ii++, a = argv[ii])
        {
            printf("%s, ", a);
        }

        printf(" WOAW.\n");
    }
    else
    {
        printf("No args here...");
    }

    // Ajout des handlers pour les signaux systemes
    main_add_handlers();

    return 0;
}


int main(int argc, char *argv[])
{
    int ret = 0;

    // Recuperation des arguments
    main_config(argc, argv);

    printf("Hello World in ARM arch\n");

    ret = main_start_factory();

    if (BASE_OK != ret)
    {
        LOG_ERR("MAIN : erreur de démarrage. EXIT (code %d)\n", ret);
    }

    // Arret du système
    ret = main_stop_factory();

    return ret;
}
