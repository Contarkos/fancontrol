#pragma once

// Includes globaux
#include <poll.h>

// Includes locaux
#include "com.h"
#include "remote.h"

// Define
#define REMOTE_MODULE_NAME      "REMOTE"
#define REMOTE_POLL_TIMEOUT     1000

typedef enum
{
    REMOTE_FD_SOCKET = 0,
    REMOTE_FD_UDP = 1,
    REMOTE_FD_NB
} t_remote_fd_index;

class REMOTE : public MODULE
{
    public:
        REMOTE(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod);
        REMOTE();
        ~REMOTE();

    private:
        struct pollfd p_fd[REMOTE_FD_NB];  // Structure pour polling
        int timer_fd;                   // Index du timer requested
        int timeout_fd;                 // File descriptor donné au timer pour envoyer les messages de timeout
        int socket_fd;                  // File descriptor pour recevoir les messages
        int udp_fd;                     // File descriptor pour recevoir les interruptions

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
        // Methodes de constructeurs
        void remote_init_pollfd();

        // Recuperation des donnees
        int remote_treat_msg(int i_fd);

};