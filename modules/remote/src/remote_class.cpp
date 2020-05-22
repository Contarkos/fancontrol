/* Includes globaux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

/* Includes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "remote.h"
#include "remote_class.h"

/*****************************************************************************/
/*                               Defines                                     */
/*****************************************************************************/

//#define REMOTE_UDP_KO 1

#define REMOTE_LOCAL_ADDR       COM_LOCAL_IP_ADDR
#define REMOTE_LOCAL_PORT       41001

/*****************************************************************************/
/* Variables globales */
/*****************************************************************************/

static t_uint32 remote_msg_array[] =
{
    MAIN_START,
    MAIN_SHUTDOWN,
    REMOTE_TIMER,
};

static t_uint32 remote_msg_array_size = sizeof(remote_msg_array) / sizeof(remote_msg_array[0]);

/*****************************************************************************/
/* Definition des constructeurs */
/*****************************************************************************/

REMOTE::REMOTE(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod) : MODULE(mod_name, m_main, m_mod)
{
    this->remote_init_pollfd();
}

REMOTE::REMOTE() : MODULE()
{
    this->remote_init_pollfd();
}

void REMOTE::remote_init_pollfd()
{
    int ii;

    for (ii = 0; ii < REMOTE_FD_NB; ii++)
    {
        this->p_fd[ii].fd = -1;
        this->p_fd[ii].events = POLLIN;
    }
}

REMOTE::~REMOTE()
{
    ;
}

int REMOTE::start_module()
{
    int ret = 0;
    char s[] = REMOTE_SOCKET_NAME;

    /* Init timer regulier */
    this->timer_fd = COM_create_timer_msg(REMOTE_TIMER_USEC, OS_TIMER_PERIODIC, REMOTE_TIMER);

    if (this->timer_fd < 0)
    {
        LOG_ERR("REMOTE : error while creating loop timer");
        ret = -1;
    }
    else
    {
        /* Ouverture socket UNIX */
        this->socket_fd = COM_create_socket(AF_UNIX, SOCK_DGRAM, 0, s, sizeof(s));

        if (this->socket_fd > 0)
        {
            LOG_INF3("REMOTE : creation socket OK, fd = %d", this->socket_fd);
            this->p_fd[REMOTE_FD_SOCKET].fd = this->socket_fd;
        }
        else
        {
            LOG_ERR("REMOTE : error creating socket");
            ret += -2;
        }
    }

    if (0 == ret)
    {
        /* Register the module */
        ret = COM_msg_register(COM_ID_REMOTE, &this->remote_semfd);

        if (0 != ret)
        {
            LOG_ERR("REMOTE : error while registering the module in COM, ret = %d", ret);
            ret = -8;
        }
        else
        {
            this->p_fd[REMOTE_FD_COM].fd = this->remote_semfd;

            ret = COM_msg_subscribe_array(COM_ID_REMOTE, remote_msg_array, remote_msg_array_size);
        }
    }

#ifndef REMOTE_UDP_KO
    if (0 == ret)
    {
        /* Configuration de la socket UDP de sortie */
        struct sockaddr_in multi_addr;
        multi_addr.sin_family = AF_INET;
        multi_addr.sin_port = htons(REMOTE_MULTICAST_PORT);
        inet_aton(REMOTE_MULTICAST_ADDR, &multi_addr.sin_addr);

        //ret = COM_connect_socket(AF_INET, SOCK_DGRAM, (char *) &multi_addr, sizeof(multi_addr), &this->udp_fd);
        ret = COM_create_mcast_socket(&this->out_socket, REMOTE_LOCAL_ADDR, REMOTE_LOCAL_PORT, REMOTE_MULTICAST_ADDR, REMOTE_MULTICAST_PORT);

        if (0 == ret)
        {
            LOG_INF3("REMOTE : creation socket UDP OK, fd = %d", this->out_socket.fd);
            this->p_fd[REMOTE_FD_UDP].fd = this->out_socket.fd;
        }
        else
        {
            LOG_ERR("REMOTE : error creating socket UDP, ret = %d", ret);
            /* TODO : fermer proprement la socket sur erreur */
            ret += -4;
        }
    }
#endif

    return ret;
}

/* Initialisation après l'init de tous les modules */
int REMOTE::init_after_wait(void)
{
    int ret = 0;

    /* Demarrage du timer REMOTE */
    if (0 == ret)
    {
        ret = OS_start_timer(this->timer_fd);

        if (ret != 0)
            LOG_ERR("REMOTE : timer not started, ret = %d", ret);
    }

    return ret;
}

/* Arret spécifique à ce module */
int REMOTE::stop_module()
{
    int ret = 0;

    /* Arret du timer */
    ret += OS_stop_timer(this->timer_fd);

    /* Fermeture de la socket */
    ret += COM_close_socket(this->socket_fd);

#ifndef REMOTE_UDP_KO
    /* Fermeture socket UDP */
    ret += COM_close_socket(this->out_socket.fd);
#endif

    return ret;
}

int REMOTE::exec_loop()
{
    int ret = 0, read_fd = 0, ii;

    /* Ecoute sur les fd */
    read_fd = poll(this->p_fd, REMOTE_FD_NB, REMOTE_POLL_TIMEOUT);

    /* Test de non timeout */
    if (read_fd <= 0)
    {
        /* Timeout expiré */
        LOG_WNG("REMOTE : expired timeout poll");
        ret = 1;
    }
    else
    {
        for (ii = 0; ii < REMOTE_FD_NB; ii++)
        {
            if (POLLIN & this->p_fd[ii].revents)
            {
                switch (ii)
                {
                    case REMOTE_FD_SOCKET:
                        ret = remote_treat_msg(this->p_fd[ii].fd);
                        break;
                    case REMOTE_FD_UDP:
                        ret = remote_treat_udp(this->p_fd[ii].fd);
                        break;
                    case REMOTE_FD_COM:
                        ret = remote_treat_com();
                        break;
                    case REMOTE_FD_NB:
                    default:
                        LOG_WNG("REMOTE : wrong file descriptor");
                        ret = 2;
                        break;
                }
            }
        }
    }

    return ret;
}
