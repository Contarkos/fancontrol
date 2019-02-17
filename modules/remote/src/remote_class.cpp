// Includes globaux
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>
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
REMOTE::REMOTE(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod) : MODULE(mod_name, m_main, m_mod)
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

    return ret;
}

// Initialisation après l'init de tous les modules
int REMOTE::init_after_wait(void)
{
    int ret = 0;

    return ret;
}

// Arret spécifique à ce module
int REMOTE::stop_module()
{
    int ret = 0;

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
        LOG_WNG("REMOTE : timeout poll expiré");
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
                        break;
                    case REMOTE_FD_UDP:
                        break;
                    case REMOTE_FD_NB:
                    default:
                        LOG_WNG("REMOTE : mauvais file descriptor");
                        ret = 2;
                        break;
                }
            }
        }
    }

    return ret;
}
