/* Includes globaux */
#include <poll.h>

/* Includes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module_bis.h"

#include "remote.h"
#include "remote_module.h"

/*********************************************************************/
/*                               Defines                             */
/*********************************************************************/

//#define REMOTE_UDP_KO 1

#define REMOTE_LOCAL_ADDR       COM_LOCAL_IP_ADDR
#define REMOTE_LOCAL_PORT       41001

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
int remote_timer_id = -1;
int remote_irq_fd = -1;
int remote_sem_fd = -1;

struct pollfd remote_poll_fd[REMOTE_FD_NB] = 
{
    /* IRQ file descriptor */
    { .fd = -1, .events = POLLIN, },
    /* COM file descriptor */
    { .fd = -1, .events = POLLIN, }
};

t_com_socket remote_out_socket;        /* Multicast data struct */

/*********************************************************************/
/*                          Local variables                          */
/*********************************************************************/

static t_uint32 remote_msg_array[] =
{
    MAIN_START,
    MAIN_SHUTDOWN,
    REMOTE_TIMER,
};

static t_uint32 remote_msg_array_size = sizeof(remote_msg_array) / sizeof(remote_msg_array[0]);

/*********************************************************************/
/*                             Functions                             */
/*********************************************************************/

int remote_start_module (void)
{
    int ret = 0;

    /* Creating timer for the loop */
    if (0 == ret)
    {
        remote_timer_id = COM_create_timer_msg(REMOTE_TIMER_USEC, OS_TIMER_PERIODIC, REMOTE_TIMER);

        if (remote_timer_id < 0)
        {
            LOG_ERR("REMOTE : error while creating loop timer");
            ret = -1;
        }
    }

    /* Register the module */
    if (0 == ret)
    {
        ret = COM_msg_register(COM_ID_REMOTE, &remote_sem_fd);

        if (0 != ret)
        {
            LOG_ERR("REMOTE : error while registering the module in COM, ret = %d", ret);
            ret = -8;
        }
        else
        {
            remote_poll_fd[REMOTE_FD_COM].fd = remote_sem_fd;

            ret = COM_msg_subscribe_array(COM_ID_REMOTE, remote_msg_array, remote_msg_array_size);
        }
    }

#ifndef REMOTE_UDP_KO
    if (0 == ret)
    {
        /* Output UDP socket configuration */
        ret = COM_create_mcast_socket(&remote_out_socket, REMOTE_LOCAL_ADDR, REMOTE_LOCAL_PORT, REMOTE_MULTICAST_ADDR, REMOTE_MULTICAST_PORT);

        if (0 == ret)
        {
            LOG_INF3("REMOTE : creation socket UDP OK, fd = %d", remote_out_socket.fd);
            remote_poll_fd[REMOTE_FD_UDP].fd = remote_out_socket.fd;
        }
        else
        {
            LOG_ERR("REMOTE : error creating socket UDP, ret = %d", ret);
            /* TODO : properly close socket on error */
            ret += -4;
        }
    }
#endif

    return ret;
}

int remote_init_after_wait (void)
{
    int ret = 0;

    /* Starting timer */
    if (0 == ret)
    {
        ret = OS_start_timer(remote_timer_id);

        if (ret != 0)
            LOG_ERR("REMOTE : timer not started, ret = %d", ret);
    }


    return ret;
}

int remote_exec_loop (void)
{
    int ret = 0, read_fd = 0, ii;

    if (0 == ret)
    {
        /* Listen on all file descriptors */
        read_fd = poll(remote_poll_fd, REMOTE_FD_NB, REMOTE_POLL_TIMEOUT);

        /* Test on timeout */
        if (read_fd <= 0)
        {
            /* Expired timeout */
            LOG_WNG("REMOTE : expired timeout poll");
            ret = 1;
        }
        else
        {
            for (ii = 0; ii < REMOTE_FD_NB; ii++)
            {
                if (POLLIN & remote_poll_fd[ii].revents)
                {
                    switch (ii)
                    {
                        case REMOTE_FD_UDP:
                            ret = remote_treat_udp(remote_poll_fd[ii].fd);
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
    }

    return ret;
}

int remote_stop_module()
{
    int ret = 0;

    /* Stopping timer */
    ret += OS_stop_timer(remote_timer_id);

#ifndef REMOTE_UDP_KO
    /* Closing UDP socket */
    ret += COM_close_socket(remote_out_socket.fd);
#endif

    return ret;
}

