// Global includes
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

// Local includes
#include "com.h"
#include "com_msg.h"
#include "com_socket.h"

/*********************************************************************/
/*                        Variables globales                         */
/*********************************************************************/

// Listes des socket à prévenir par message
t_com_msg_list com_list_msg[COM_TOTAL_MSG];

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Creation et binding d'une socket
int COM_create_socket(int i_family, int i_type, int i_proto, char *i_data)
{
    int fd = 0, ret = 0;

    // Creation de la socket
    fd = socket(i_family, i_type, i_proto);

    if (fd <= 0)
    {
        printf("[ER] COM : erreur à la création de la socket\n");
    }
    else
    {
        // On remplit la structure sockaddr selon le type choisi
        switch (i_family)
        {
            case AF_UNIX:
                ret = com_bind_socket_unix(fd, i_data);
                break;
            case AF_INET:
                ret = com_bind_socket_inet(fd, i_data);
                break;
            default:
                printf("[ER] COM : erreur type de socket\n");
                ret = -1;
        }

        if (ret != 0)
        {
           printf("[ER] COM : erreur binding de la socket, erreur = %d\n", errno);
           fd = -1;
        }
        else
        {
            ;
        }
    }

    return fd;
}

// TODO gestion des protocoles
int COM_connect_socket(int i_family, int i_type, char * i_data, int *o_fd)
{
    int ret = 0;

    // Creation de la socket
    *o_fd = socket(i_family, i_type, 0);

    if ( *o_fd < 0 )
    {
        printf("[ER] COM : erreur à la création de la socket\n");
        ret = -1;
    }
    else
    {
        // Connect en fonction de la family
        switch (i_family)
        {
            case AF_UNIX:
                ret = com_connect_unix(*o_fd, i_data);
                break;
            case AF_INET:
                ret = com_connect_inet(*o_fd, i_data);
                break;
            default:
                printf("[ER] COM : famille de socket non gérée, famille = %d\n", i_family);
                ret = -1;
        }
    }

    return ret;
}

int COM_socket_listen(int i_fd, int i_backlog)
{
    int ret = 0;

    if ( -1 == i_fd )
    {
        printf("[ER] COM : mauvais fd pour socket sur listen\n");
        ret = -2;
    }
    else if ( 0 >= i_backlog )
    {
        printf("[ER] COM : mauvaise valeur pour le backlog, bl = %d\n", i_backlog);
        ret = -4;
    }
    else
    {
        ret = listen(i_fd, i_backlog);

        if (-1 == ret)
        {
            printf("[ER] COM : erreur sur listen de la socket, errno = %d\n", errno);
        }
    }

    return ret;
}

// Envoi d'un message avec l'ID définie dans com_msg.h
int COM_send_data(int i_fd, t_uint32 i_id, void * i_data, size_t i_size, int i_flags)
{
    int ret = 0;
    size_t s;
    t_com_msg m;

    if (i_fd < 0)
    {
        printf("[ER] COM : pas de socket valide pour envoyer les données\n");
        ret = -1;
    }
    else if ( (0 == i_size) || !(i_data) )
    {
        printf("[ER] COM : données invalides pour socket\n");
        ret = -2;
    }
    else
    {
        // Mise en forme du paquet pour les données
        m.id = i_id;
        memcpy(&(m.data), i_data, i_size);
        s = sizeof(t_uint32) + i_size;

        // Envoi des données
        ret = send(i_fd, &m, s, i_flags);

        if (ret < 0)
        {
            printf("[ER] COM : erreur d'envoi des données, errno = %d\n", errno);
        }
    }

    return ret;
}

// Reception d'un message au travers socket UDP
int COM_receive_data(int i_sock, t_com_msg *o_m, int *o_size)
{
    int ret = 0;
    char data[COM_MAX_SIZE_DATA];

    if (i_sock < 0)
    {
        printf("[ER] COM : pas de socket pour reception de message\n");
        ret = -1;
    }
    else if (!o_m)
    {
        printf("[ER] COM : pas de structure de message de sortie\n");
        ret = -2;
    }
    else
    {
        ret = recv(i_sock, data, COM_MAX_SIZE_DATA, 0);

        if (-1 == ret)
        {
           printf("[ER] COM : erreur à la recupération des données, errno = %d\n", errno);
           ret = -4;
        }
        else
        {
            memcpy(o_m, data, COM_MAX_SIZE_DATA);
            *o_size = ret;

            ret = 0;
        }
    }

    return ret;
}

int COM_register_socket(int i_fd, int *i_list, int i_size)
{
    int ret = 0, ii;

    if (i_size > COM_MAX_NB_MSG)
    {
        printf("[ER] COM : Trop de messages à écouter, nb = %d\n", i_size);
        ret = -1;
    }
    else
    {
        // Creation du tableau pour stocker les messages à ecouter

        // Ajout des éléments là où il faut
        for (ii = 0; ii < i_size; ii++)
        {
            com_add_fd_to_list(i_fd, *(i_list + ii));
        }
    }

    return ret;
}

int COM_close_socket(int i_fd)
{
    int ret = 0;

    if (i_fd)
    {
        ret = close(i_fd);
    }
    else
    {
        printf("[ER] COM : mauvaise référence au fichier, fd = %d\n", i_fd);
        ret = -1;
    }

    return ret;
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

    ret = bind(fd, (struct sockaddr *) &a, sizeof(a));

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

    ret = bind(fd, (struct sockaddr *) &a, sizeof(a));

    return ret;
}

int com_connect_unix(int fd, char *data)
{
    int ret = 0;
    struct sockaddr_un a;

    // Init des parametres de la socket
    a.sun_family = AF_UNIX;
    strncpy(a.sun_path, data, COM_UNIX_PATH_MAX);

    printf("[IS] COM : nom de la socket = %s\n", a.sun_path);

    ret = connect(fd, (struct sockaddr *) &a, sizeof(a));

    return ret;
}

int com_connect_inet(int fd, char *data)
{
    int ret = 0;
    struct sockaddr_in a;

    // Init des parametres de la socket
    a.sin_family = AF_INET;
    a.sin_port = ((t_com_inet_data *)data)->port;
    a.sin_addr.s_addr = ((t_com_inet_data *)data)->addr;

    ret = connect(fd, (struct sockaddr *) &a, sizeof(a));

    return ret;
}

int com_add_fd_to_list(int i_fd, int i_id)
{
    int ret = 0;

    return ret;
}
