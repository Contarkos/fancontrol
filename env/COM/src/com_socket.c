// Global includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>

// Local includes
#include "com.h"

// Creation et binding d'une socket
int COM_create_socket(int i_family, int i_type, int i_proto)
{
    int fd = 0, ret = 0;
    struct sockaddr a;

    fd = socket(i_family, i_type, i_proto);

    if (fd <= 0)
    {
        printf("[ER] COM : erreur à la création de la socket\n");
    }
    else
    {
        // On remplit la structure sockaddr selon le type choisi
        switch (i_type)
        {
            case AF_UNIX:
                ((struct sockaddr_un *) &a)->sun_family = AF_UNIX;
                break;
            case AF_INET:
                ((struct sockaddr_in *) &a)->sin_family = AF_INET;
                break;
            default:
                printf("[ER] COM : erreur type de socket\n");
        }

        // Binding de la socket
        ret = bind(fd, &a, sizeof(struct sockaddr));

        if (ret != 0)
        {
           printf("[ER] COM : erreur binding de la socket\n");
           fd = 0;
        }
        else
        {
            ;
        }
    }

    return fd;
}
