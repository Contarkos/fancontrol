#pragma once

/* Includes globaux */
#include <poll.h>

/* Includes locaux */
#include "com.h"
#include "remote.h"
#include "module.h"

/* Define */
#define REMOTE_MODULE_NAME      "REMOTE"
#define REMOTE_POLL_TIMEOUT     1000

#define REMOTE_TIMER_USEC       500000

typedef enum
{
    REMOTE_FD_SOCKET = 0,
    REMOTE_FD_UDP = 1,
    REMOTE_FD_COM = 2,
    REMOTE_FD_NB
} t_remote_fd_index;

class REMOTE : public MODULE
{
    public:
        REMOTE(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod);
        REMOTE();
        ~REMOTE();

    private:
        struct pollfd p_fd[REMOTE_FD_NB];  /* Structure pour polling */
        int timer_fd;                   /* Index du timer requested */
        int socket_fd;                  /* File descriptor pour recevoir les messages */
        int remote_semfd;               /* File descriptor pour recevoir les messages de la queue */
        t_com_socket out_socket;        /* Structure contenant les donnees pour le multicast */

        /***********************************************/
        /*             Methodes virtuelles             */
        /***********************************************/
        int start_module(void);
        int stop_module(void);

        int init_after_wait(void);
        int exec_loop(void);

        /***********************************************/
        /*             Methodes spécifiques            */
        /***********************************************/

        /* Methodes de constructeurs */
        void remote_init_pollfd();

        /* Recuperation des donnees */
        int remote_treat_msg(int i_fd);
        int remote_treat_com(void);
        int remote_treat_udp(int i_fd);

        /* Traitement des messages */
        int remote_send_status(void);
};
