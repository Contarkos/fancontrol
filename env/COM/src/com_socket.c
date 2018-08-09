// Global includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>

#include <string.h>

// Local includes
#include "com.h"
#include "com_socket.h"

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Creation et binding d'une socket
int COM_create_socket(int i_family, int i_type, int i_proto, char *i_data)
{
    int fd = 0, ret = 0;

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
                ret = com_bind_socket_unix(fd, i_data);
                break;
            case AF_INET:
                ret = com_bind_socket_inet(fd, i_data);
                break;
            default:
                printf("[ER] COM : erreur type de socket\n");
        }

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

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

int com_bind_socket_unix(int fd, char *data)
{
    int ret = 0;
    struct sockaddr_un a;

    // Init des parametres de la socket
    a.sun_family = AF_UNIX;
    strncpy(a.sun_path, data, COM_UNIX_PATH_MAX);

    ret = bind(fd, (struct sockaddr *) &a, sizeof(struct sockaddr));

    return ret;
}

int com_bind_socket_inet(int fd, char *data)
{
    int ret = 0;
    struct sockaddr_in a;

    // Init des parametres de la socket
    a.sin_family = AF_INET;
    a.sin_port = ((t_com_inet_data *)data)->port;
    a.sin_addr.s_addr = ((t_com_inet_data *)data)->addr;

    ret = bind(fd, (struct sockaddr *) &a, sizeof(struct sockaddr));

    return ret;
}
