#ifndef REMOTE_MODULE_H_
#define REMOTE_MODULE_H_

/* Global includes */
#include <poll.h>

/* Local includes */
#include "com.h"
#include "module_bis.h"

#include "remote.h"

/*********************************************************************/
/*                           Defines                                 */
/*********************************************************************/

#define REMOTE_MODULE_NAME      "REMOTE"
#define REMOTE_POLL_TIMEOUT     1000

#define REMOTE_TIMER_USEC       500000

typedef enum
{
    REMOTE_FD_UDP = 0,
    REMOTE_FD_COM = 1,
    REMOTE_FD_NB
} t_remote_fd_index;

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
extern t_mod_context remote_modules[NB_INSTANCES_REMOTE];

extern int remote_timer_id;
extern int remote_sem_fd;

extern struct pollfd remote_poll_fd[REMOTE_FD_NB];

extern t_com_socket remote_out_socket;        /* Multicast data struct */

/*********************************************************************/
/*                      Internal functions                           */
/*********************************************************************/

/* Functions for handling the module */
int remote_start_module    (void);
int remote_stop_module     (void);

int remote_init_after_wait (void);
int remote_exec_loop       (void);

/* Data handling entry points */
int remote_treat_com(void);
int remote_treat_udp(int i_fd);

/* Messages treatment */
int remote_send_status(void);
#endif /* REMOTE_MODULE_H_ */

