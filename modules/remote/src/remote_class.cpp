// Includes globaux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "remote.h"
#include "remote_class.h"

// Variables globales

// Definition des constructeurs
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

    // Init timer regulier
    this->timer_fd = OS_create_timer(REMOTE_TIMER_USEC, &REMOTE::remote_timer_handler, OS_TIMER_PERIODIC, (void *) this);

    if (this->timer_fd < 0)
    {
        LOG_ERR("REMOTE : error while creating loop timer");
        ret = -1;
    }
    else
    {
        // Ouverture socket UNIX
        this->socket_fd = COM_create_socket(AF_UNIX, SOCK_DGRAM, 0, s);

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

        // Ouverture socket UDP
        t_com_inet_data d;
        d.addr = INADDR_ANY;
        d.port = 31001;

        this->udp_fd = COM_create_socket(AF_INET, SOCK_DGRAM, 0, (char *) &d);

        if (this->udp_fd > 0)
        {
            LOG_INF3("REMOTE : creation socket UDP OK, fd = %d", this->socket_fd);
            this->p_fd[REMOTE_FD_UDP].fd = this->udp_fd;
        }
        else
        {
            LOG_ERR("REMOTE : error creating socket UDP");
            ret += -4;
        }
    }

    return ret;
}

// Initialisation après l'init de tous les modules
int REMOTE::init_after_wait(void)
{
    int ret = 0;
    char t[] = REMOTE_SOCKET_NAME;

    // Connexion a la socket temp pour envoyer le message de timer
    ret = COM_connect_socket(AF_UNIX, SOCK_DGRAM, t, &(this->timeout_fd));

    if (ret != 0)
    {
        LOG_ERR("REMOTE : timer not started, ret = %d", ret);
    }
    else
    {
        // Demarrage du timer REMOTE
        ret = OS_start_timer(this->timer_fd);
    }

    return ret;
}

// Arret spécifique à ce module
int REMOTE::stop_module()
{
    int ret = 0;

    // Arret du timer
    ret += OS_stop_timer(this->timer_fd);

    // Fermeture de la socket
    ret += COM_close_socket(this->socket_fd);

    // Fermeture socket timeout
    ret += COM_close_socket(this->timeout_fd);

    return ret;
}

int REMOTE::exec_loop()
{
    int ret = 0, read_fd = 0, ii;

    // Ecoute sur les fd
    read_fd = poll(this->p_fd, REMOTE_FD_NB, REMOTE_POLL_TIMEOUT);

    // Test de non timeout
    if (read_fd <= 0)
    {
        // Timeout expiré
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
