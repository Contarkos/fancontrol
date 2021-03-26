// Module pour lire les commandes de l'utilisateur

// Global includes
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <poll.h>

// Local includes
#include "main.h"
#include "cmd.h"
#include "cmd_parse.h"
#include "integ_log.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

regex_t reg_quit;
regex_t reg_msg;

struct pollfd cmd_ufds[CMD_NB_FD] = {
    { .fd = STDIN_FILENO, .events = POLLIN }
};

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

int CMD_init(void)
{
    int ret = 0;

    // Init des regex avec regcomp

    return ret;
}

int CMD_read(void)
{
    int ret = 0, ret_read, read_fds, ii;
    char line[CMD_MAX_SIZE];

    static int is_new_line = 1;

    // Affichage du prompt
    if (is_new_line)
    {
        is_new_line = 0;

        printf("fan-app> ");
    }

    read_fds = poll(cmd_ufds, CMD_NB_FD, CMD_POLL_TIMEOUT);

    if (read_fds <= 0)
    {
        // Pas de commandes reçues
        is_new_line = 0;
    }
    else
    {
        for(ii = 0; ii < CMD_NB_FD; ii++)
        {
            if (cmd_ufds[ii].revents & POLLIN)
            {
                // Lecture de la ligne
                ret_read = read(cmd_ufds[ii].fd, line, sizeof(line));

                if (0 == ret_read)
                {
                    LOG_ERR("CMD : pas de ligne récupérée");
                    ret = -1;
                }
                else
                {
                    // Parsing et traitement
                    ret = cmd_parse_and_exec(line);

                    is_new_line = 1;
                }

            }
        }

    }

    return ret;
}

int CMD_stop(void)
{
    int ret = 0;

    // Liberation des regex avec regfree

    return ret;
}

/*********************************************************************/
/*                       Fonctions internes                          */
/*********************************************************************/

int cmd_parse_and_exec(char line[CMD_MAX_SIZE])
{
   int ret = 0;

   printf("Commande recue : %s\n", line);

   if (0 == strncmp(line, "quit", 4))
   {
      main_is_running = 0;
   }

   return ret;
}

