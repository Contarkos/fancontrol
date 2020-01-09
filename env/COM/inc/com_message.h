#pragma once

/*****************************************************************************/
/*                              Global includes                              */
/*****************************************************************************/

#include <stdio.h>

/*****************************************************************************/
/*                              Local includes                               */
/*****************************************************************************/

#include "os.h"
#include "com.h"

/*****************************************************************************/
/*                                Defines                                    */
/*****************************************************************************/

/*****************************************************************************/
/*                                Typedef                                    */
/*****************************************************************************/

/* Structure holding all the data about a module's queue */
typedef struct
{
    /* Status of initialisation for the queue */
    t_os_ret_okko is_init;
    /* Mutex to protect access to the queue */
    OS_mutex_t mutex;
    /* Signal number to raise when a new message is here */
    OS_semfd_t semfd;
    /* Points to the current index of the first unread message in the array */
    t_uint32 cur_index;
    /* Number of messages to be read */
    t_uint32 nb_msg;
    /* Actual list of messages */
    t_com_msg_struct list[COM_MAX_NB_MSG];
} t_com_msg_list;

/* If the ID is 0, the location in the array is free */
typedef struct s_com_msg_subscribe
{
    t_uint32 nb_subscribers;
    t_uint32 sub_list[COM_MAX_SUBSCRIBER];
} t_com_msg_subscribe;

/*****************************************************************************/
/*                            Variables globales                             */
/*****************************************************************************/

extern t_com_msg_subscribe com_list_msg[COM_TOTAL_MSG];
extern t_com_msg_list com_list_queues[COM_ID_NB];

/*****************************************************************************/
/*                              Prototypes                                   */
/*****************************************************************************/

int com_init_msg(void);
int com_stop_msg(void);
