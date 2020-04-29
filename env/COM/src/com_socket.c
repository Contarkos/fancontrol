/*****************************************************************************/
/*                               Global includes                             */
/*****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

/*****************************************************************************/
/*                                Local includes                             */
/*****************************************************************************/

#include "integ_log.h"
#include "com.h"
#include "com_msg.h"
#include "com_socket.h"

/*****************************************************************************/
/*                              Variables globales                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                Fonctions API                              */
/*****************************************************************************/

// Creation et binding d'une socket
int COM_create_socket(int i_family, int i_type, int i_proto, char *i_data, size_t i_size)
{
    int fd = 0, ret = 0;

    // Creation de la socket
    fd = socket(i_family, i_type, i_proto);

    if (fd <= 0)
    {
        LOG_ERR("COM : erreur à la création de la socket");
    }
    else
    {
        // On remplit la structure sockaddr selon le type choisi
        switch (i_family)
        {
            case AF_UNIX:
                ret = com_bind_socket_unix(fd, i_data, i_size);
                break;
            case AF_INET:
                ret = com_bind_socket_inet(fd, i_data, i_size);
                break;
            default:
                LOG_ERR("COM : erreur type de socket");
                ret = -1;
        }

        if (ret != 0)
        {
           LOG_ERR("COM : erreur binding de la socket, erreur = %d", errno);
           fd = -1;
        }
        else
        {
            ;
        }
    }

    return fd;
}

/* Creating and configuring a multicast socket */
int COM_create_mcast_socket(t_com_socket *o_socket,
                           const char *i_inaddr, t_uint16 i_inport,
                           const char *i_outaddr, t_uint16 i_outport)
{
    int ret = 0;
    int fd = 0;

    if (NULL == o_socket)
    {
        LOG_ERR("COM : null pointer for the socket");
        ret = -1;
    }

    /* Creating the initial socket */
    if (0 == ret)
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);

        if (fd < 0)
        {
            LOG_ERR("COM : error creating multicast socket, errno = %d", errno);
            ret = -1;
        }
    }

    /* Filling the output struct */
    if (0 == ret)
    {
        if (NULL != i_outaddr)
            inet_aton(i_outaddr, &o_socket->dest.sin_addr);
        else
            ret += -1;

        if (0 != i_outport)
        {
            o_socket->dest.sin_family = AF_INET;
            o_socket->dest.sin_port = htons(i_outport);
        }
        else
            ret += -1;
    }

    /* Setting the option for the socket */
    if (0 == ret)
    {
        /* Suppression de la boucle de multicast */
        char loop_conf = 0;
        ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop_conf, sizeof(loop_conf));

        if (ret < 0)
            LOG_ERR("COM : sortie de boucle multicast en erreur, ret = %d", ret);

        /* Ajout de l'interface utilisee pour envoyer les messages */
        if (NULL != i_inaddr)
        {
            inet_aton(i_inaddr, &o_socket->local.sin_addr);
            ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&o_socket->local.sin_addr, sizeof(o_socket->local.sin_addr));
        }

        if (0 != i_inport)
            o_socket->local.sin_port = htons(i_inport);
        else
            ret += -1;

        if (0 == ret)
        {
            o_socket->local.sin_family = AF_INET;
            ret = com_bind_socket_inet(fd, &o_socket->local, sizeof(struct sockaddr_in));
        }
        else
            LOG_ERR("COM : binding de l'interface locale en erreur, ret = %d", ret);
    }

    if (0 == ret)
    {
        u_char ttl = 4;
        ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    }

    if (0 == ret)
        o_socket->fd = fd;
    else
        LOG_ERR("COM : error while configuring multicast socket, ret = %d", ret);

    return ret;
}

// TODO gestion des protocoles
int COM_connect_socket(int i_family, int i_type, char * i_data, size_t i_size, int *o_fd)
{
    int ret = 0;

    // Creation de la socket
    *o_fd = socket(i_family, i_type, 0);

    if ( *o_fd < 0 )
    {
        LOG_ERR("COM : error while creating socket");
        ret = -1;
    }
    else
    {
        // Connect en fonction de la family
        switch (i_family)
        {
            case AF_UNIX:
                ret = com_connect_unix(*o_fd, i_data, i_size);
                break;
            case AF_INET:
                ret = com_connect_inet(*o_fd, i_data, i_size);
                break;
            default:
                LOG_ERR("COM : family of socket not handled, family = %d", i_family);
                ret = -2;
        }
    }

    return ret;
}

int COM_socket_listen(int i_fd, int i_backlog)
{
    int ret = 0;

    if ( -1 == i_fd )
    {
        LOG_ERR("COM : mauvais fd pour socket sur listen");
        ret = -2;
    }
    else if ( 0 >= i_backlog )
    {
        LOG_ERR("COM : mauvaise valeur pour le backlog, bl = %d", i_backlog);
        ret = -4;
    }
    else
    {
        ret = listen(i_fd, i_backlog);

        if (-1 == ret)
        {
            LOG_ERR("COM : erreur sur listen de la socket, errno = %d", errno);
        }
    }

    return ret;
}

// Envoi d'un message avec l'ID définie dans com_msg.h
int COM_send_data(int i_fd, t_uint32 i_id, void * i_data, size_t i_size, int i_flags)
{
    int ret = 0, ret_s;
    size_t s;
    t_com_msg m;

    if (i_fd < 0)
    {
        LOG_ERR("COM : pas de socket valide pour envoyer les données");
        ret = -1;
    }
    else if ( (0 == i_size) || !(i_data) )
    {
        LOG_ERR("COM : données invalides pour socket");
        ret = -2;
    }
    else
    {
        // Mise en forme du paquet pour les données
        m.id = i_id;
        memcpy(&(m.data), i_data, i_size);
        s = sizeof(t_uint32) + i_size;

        // Envoi des données
        ret_s = send(i_fd, &m, s, i_flags);

        if (ret_s < 0)
        {
            LOG_ERR("COM : erreur d'envoi des données, errno = %d", errno);
            ret = -4;
        }
        else if (s != (size_t) ret_s)
        {
            LOG_ERR("COM : erreur de taille des donnees envoyees, %d != %d", s, ret_s);
            ret = -8;
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}


// Envoi d'un message avec l'ID définie dans com_msg.h
int COM_send_mcast_data(t_com_socket *i_socket, t_uint32 i_id, void * i_data, size_t i_size, int i_flags)
{
    int ret = 0, ret_s;
    size_t s;
    t_com_msg m;

    if (NULL == i_socket)
    {
        LOG_ERR("COM : no valid socket to send the data");
        ret = -1;
    }
    else if ( (0 == i_size) || !(i_data) )
    {
        LOG_ERR("COM : invalid data for socket");
        ret = -2;
    }

    if (0 == ret)
    {
        // Mise en forme du paquet pour les données
        m.id = i_id;
        memcpy(&(m.data), i_data, i_size);
        s = sizeof(t_uint32) + i_size;

        // Envoi des données
        ret_s = sendto(i_socket->fd, &m, s, i_flags, (struct sockaddr *) &i_socket->dest, sizeof(i_socket->dest));

        if (ret_s < 0)
        {
            LOG_ERR("COM : error while sending data, errno = %d", errno);
            ret = -4;
        }
        else if (s != (size_t) ret_s)
        {
            LOG_ERR("COM : size error for the data sent, %d != %d", s, ret_s);
            ret = -8;
        }
        else
        {
            ret = 0;
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
        LOG_ERR("COM : no socket for receiving messages");
        ret = -1;
    }
    else if (!o_m)
    {
        LOG_ERR("COM : no structure for output message");
        ret = -2;
    }
    else
    {
        ret = recv(i_sock, data, COM_MAX_SIZE_DATA, 0);

        if (-1 == ret)
        {
           LOG_ERR("COM : error while retrieving data, errno = %d", errno);
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
        LOG_ERR("COM : too many messages to listen to, nb = %d", i_size);
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
        LOG_ERR("COM : bad reference to file, fd = %d", i_fd);
        ret = -1;
    }

    return ret;
}

/*****************************************************************************/
/*                             Fonctions locales                             */
/*****************************************************************************/

int com_bind_socket_unix(int fd, char *data, size_t size_data)
{
    int ret = 0;
    struct sockaddr_un a;

    // Init des parametres de la socket
    a.sun_family = AF_UNIX;
    strncpy(a.sun_path, data, size_data);

    // Suppression de l'ancienne socket
    (void)unlink(a.sun_path);

    // Binding de la socket en fonction de la taille du chemin
    ret = bind(fd, (struct sockaddr *) &a, sizeof(a.sun_family) + sizeof(a.sun_path));

    return ret;
}

int com_bind_socket_inet(int fd, void *data, size_t size_data)
{
    int ret = 0;

    ret = bind(fd, (struct sockaddr *) data, size_data);

    if (ret)
       LOG_ERR("COM : error binding INET socket, ernno = %d", errno);

    return ret;
}

int com_connect_unix(int fd, char *data, size_t size_data)
{
    int ret = 0;
    struct sockaddr_un a;

    // Init des parametres de la socket
    a.sun_family = AF_UNIX;
    strncpy(a.sun_path, data, size_data);

    LOG_INF1("COM : socket name = %s", a.sun_path);

    ret = connect(fd, (struct sockaddr *) &a, sizeof(a.sun_family) + sizeof(a.sun_path));

    return ret;
}

int com_connect_inet(int fd, void *data, size_t size_data)
{
    int ret = 0;

    ret = connect(fd, (struct sockaddr *) data, size_data);

    if (0 != ret)
       LOG_ERR("COM : error while connecting INET socket, errno = %d", errno);

    return ret;
}

int com_add_fd_to_list(int i_fd, int i_id)
{
    int ret = 0;

    return ret;
}
