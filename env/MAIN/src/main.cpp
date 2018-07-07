// Include globaux
#include <stdio.h>
#include <unistd.h>

// Include locaux
#include "main_handler.h"
#include "main_factory.h"

// Include à virer plus tard
#include "base.h"
#include "fan.h"

static int main_config(int argc, char *argv[])
{
    int ii;
    char *a;

    if (argc > 0)
    {
        printf("There is %d args which are : ", argc);

        for (ii = 0, a = argv[0]; ii < argc; ii++, a = argv[ii])
        {
            printf("%s, ", a);
        }

        printf(" you ! WOAW.\n");
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
        printf("MAIN : erreur de démarrage. EXIT (code %d)\n", ret);
    }

    ret = main_stop_factory();

    return ret;
}
