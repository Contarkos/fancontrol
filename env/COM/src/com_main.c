// Global includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Local includes
#include "com.h"
#include "com_msg.h"
#include "com_socket.h"

// Variables globales
int com_extern_socket = -1;

int COM_init(void)
{
    int ret = 0;
    t_com_inet_data d = { .addr = INADDR_ANY, .port = 21001 }; // addr = toutes les addresses

    // Ouverture de la socket vers l'exterieur
    com_extern_socket = COM_create_socket(AF_INET, SOCK_STREAM, 0, (char *) &d);

    if (-1 == com_extern_socket)
    {
        printf("[ER] COM : erreur création socket externe\n");
        ret = -1;
    }
    else
    {
        // Ecoute sur le bon port
        ret = COM_socket_listen(com_extern_socket, COM_EXTERN_BACKLOG);
    }

    return ret;
}

int COM_stop(void)
{
    int ret = 0;

    ret = COM_close_socket(com_extern_socket);

    return ret;
}
