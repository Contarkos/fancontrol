/*****************************************************************************/
/*                               Global includes                             */
/*****************************************************************************/

#include <string.h>

/*****************************************************************************/
/*                                Local includes                             */
/*****************************************************************************/

#include "integ_log.h"
#include "com.h"
#include "com_msg.h"
#include "com_socket.h"

#include "com_message.h"

/*****************************************************************************/
/*                                Global variables                           */
/*****************************************************************************/

/* List of subscribers for each message */
t_com_msg_subscribe com_list_msg[COM_TOTAL_MSG];

/* Array of queues for each modules */
t_com_msg_list com_list_queues[COM_ID_NB];

/*****************************************************************************/
/*                                Functions API                              */
/*****************************************************************************/

int COM_msg_subscribe(t_com_id_modules i_module, t_uint32 i_msg)
{
    int ret = 0;
    t_com_msg_subscribe *msg_sub;

    if (COM_TOTAL_MSG < i_msg)
    {
        LOG_ERR("COM : wrong id for message, id = %d", i_msg);
        ret = -1;
    }

    if (0 == ret)
    {
        msg_sub = &com_list_msg[i_msg];

        if (COM_MAX_SUBSCRIBER < msg_sub->nb_subscribers)
        {
            LOG_ERR("COM : too many subscribers for msg %d", i_msg);
            ret = -2;
        }
    }

    if (0 == ret)
    {
        switch (i_module)
        {
            case COM_ID_MAIN:
            case COM_ID_OS:
            case COM_ID_COM:
            case COM_ID_CMD:
            case COM_ID_MODULE:
            case COM_ID_FAN:
            case COM_ID_TEMP:
            case COM_ID_REMOTE:
                break;
            case COM_ID_NULL:
            case COM_ID_NB:
            default:
                LOG_ERR("COM : wrong ID for module during subscription, module = %d", i_module);
                ret = -4;
                break;
        }
    }

    /* See if the module already subscribed to this message */
    if (0 == ret)
    {
        t_uint32 ii = 0;

        while (ii < msg_sub->nb_subscribers)
        {
            if (msg_sub->sub_list[ii] == i_module)
            {
                LOG_WNG("COM : module %d already subscribed to %d", i_module, i_msg);
                ret = 1;
                break;
            }

            ii++;
        }
    }

    /* Everything is ok and the module did not subscribe */
    if (0 == ret)
    {
        msg_sub->sub_list[msg_sub->nb_subscribers] = i_module;
        msg_sub->nb_subscribers++;
    }

    return ret;
}

/* Add multiple messages at once. */
int COM_msg_subscribe_array(t_com_id_modules i_module, t_uint32* i_msg_array, t_uint32 i_size)
{
    int ret = 0;
    t_uint32 ii;

    for (ii = 0; ii < i_size; ii++)
    {
        ret = COM_msg_subscribe(i_module, i_msg_array[ii]);

        if (0 > ret)
        {
            LOG_ERR("COM : error while subscribing multiple messages, last index = %d", ii);
            break;
        }
    }

    return ret;
}

int COM_msg_unsub(t_com_id_modules i_module, t_uint32 i_msg)
{
    int ret = 0;
    t_uint32 ii = 0;
    t_com_msg_subscribe *msg_sub;

    if (COM_TOTAL_MSG < i_msg)
    {
        LOG_ERR("COM : wrong id for message, id = %d", i_msg);
        ret = -1;
    }

    if (0 == ret)
    {
        msg_sub = &com_list_msg[i_msg];

        if (0 == msg_sub->nb_subscribers)
        {
            LOG_ERR("COM : no subscribers for message %d", i_msg);
            ret = -2;
        }
    }

    if (0 == ret)
    {
        switch (i_module)
        {
            case COM_ID_MAIN:
            case COM_ID_OS:
            case COM_ID_COM:
            case COM_ID_CMD:
            case COM_ID_MODULE:
            case COM_ID_FAN:
            case COM_ID_TEMP:
            case COM_ID_REMOTE:
                break;
            case COM_ID_NULL:
            case COM_ID_NB:
            default:
                LOG_ERR("COM : wrong ID for module during subscription, module = %d", i_module);
                ret = -4;
                break;
        }
    }

    /* Check whether the module already subscribed to the message */
    if (0 == ret)
    {
        while (ii < msg_sub->nb_subscribers)
        {
            if (msg_sub->sub_list[ii] == i_module)
                break; /* We found it */

            ii++;
        }

        /* We did not find it */
        if (ii == msg_sub->nb_subscribers)
            ret = 1;
    }

    /* Everything is ok and we need to remove the module from the list */
    if (0 == ret)
    {
        /* We put the last in the list where the removed module was */
        msg_sub->sub_list[ii] = msg_sub->sub_list[msg_sub->nb_subscribers - 1];
        msg_sub->sub_list[msg_sub->nb_subscribers - 1] = COM_ID_NULL;

        msg_sub->nb_subscribers--;
    }

    return ret;
}

int COM_msg_unsub_array(t_com_id_modules i_module, t_uint32 *i_msg, t_uint32 i_size)
{
    int ret = 0;
    t_uint32 ii;

    for (ii = 0; ii < i_size; ii++)
    {
        ret = COM_msg_unsub(i_module, i_msg[ii]);

        if (0 > ret)
        {
            LOG_ERR("COM : error while unsubscribing to multiple messages, module = %d, ii = %d", i_module, ii);
            break;
        }
    }

    return ret;
}

/*
 * Send the message with the ID i_msg to all the modules who subscribed to
 * it.
 * If i_size is larger than COM_MAX_SIZE_DATA, the data is silently truncated.
 */
int COM_msg_send(t_uint32 i_msg, void* i_data, t_uint32 i_size)
{
    int ret = 0;

    t_uint32 ii;
    t_uint32 id_module;
    t_uint32 copy_size;

    t_com_msg_subscribe *msg_sub;
    t_com_msg_list *msg_queue;

    if (i_msg > COM_TOTAL_MSG)
    {
        LOG_ERR("COM : wrong id for message, id = %d", i_msg);
        ret = -1;
    }

    if (0 == ret)
    {
        msg_sub = &com_list_msg[i_msg];

        if (0 == msg_sub->nb_subscribers)
        {
            LOG_WNG("COM : no subscribers for message %d", i_msg);
            ret = 1;
        }
    }

    if (0 == ret)
    {
        if (i_size > COM_MAX_SIZE_DATA)
            copy_size = COM_MAX_SIZE_DATA;
        else
            copy_size = i_size;

        if (!i_data)
        {
            LOG_ERR("COM : bad pointer to data");
            ret = -4;
        }
    }

    /* Copy the data in each queue */
    if (0 == ret)
    {
        for (ii = 0; ii < msg_sub->nb_subscribers; ii++)
        {
            /* Get the ID of the current module to send the message to */
            id_module = msg_sub->sub_list[ii];
            msg_queue = &com_list_queues[id_module];

            /* Take the mutex to avoid writing 2 messages to the same index */
            ret = OS_mutex_lock(&msg_queue->mutex);

            if (0 != ret)
            {
                LOG_ERR("COM : error while locking mutex for queue %d", id_module);
            }
            else
            {
                /* Actual copy is here */
                t_uint32 copy_index = (msg_queue->cur_index + msg_queue->nb_msg) % COM_MAX_NB_MSG;
                t_com_msg_struct *msg = &msg_queue->list[copy_index];

                /* FIXME : add the size of the data in the structure */
                OS_gettime(&msg->header.timestamp_s, &msg->header.timestamp_ns);
                msg->header.id = i_msg;
                msg->header.size = copy_size;

                memcpy(msg->body, i_data, copy_size);

                if (msg_queue->nb_msg < COM_MAX_NB_MSG)
                    msg_queue->nb_msg = (msg_queue->nb_msg + 1);

                /* Release the mutex */
                ret = OS_mutex_unlock(&msg_queue->mutex);
            }
        }
    }

    /* Signal each module that a message is here */
    if (0 == ret)
    {
        for (ii = 0; (ii < msg_sub->nb_subscribers) && (0 == ret); ii++)
        {
            /* Get the ID of the current module to send the message to */
            id_module = msg_sub->sub_list[ii];
            msg_queue = &com_list_queues[id_module];

            if (OS_RET_OK == msg_queue->is_init)
                ret = OS_semfd_post(&msg_queue->semfd);
            else
                ret = -8;

            if (ret < 0)
               LOG_ERR("COM : error on posting message, ii = %d, ret = %d", ii, ret);
        }
    }

    return ret;
}

/*
 * Copy an unread message from the queue of the <i_module> module
 * in the buffer behind <o_msg>
 */
int COM_msg_read(t_com_id_modules i_module, t_com_msg_struct *o_msg)
{
    int ret = 0;
    t_com_msg_list *msg_queue;

    /* Check on ID module */
    if (i_module >= COM_ID_NB)
    {
        LOG_ERR("COM : wrong id for module, id = %d", i_module);
        ret = -1;
    }

    if (0 == ret)
    {
        msg_queue = &com_list_queues[i_module];

        if (0 == msg_queue->nb_msg)
        {
            LOG_WNG("COM : no message to read in the queue of module n°%d", i_module);
            ret = 1;
        }
    }

    if (0 == ret)
    {
        ret = OS_semfd_wait(&msg_queue->semfd);

        if (0 != ret)
            LOG_ERR("COM : error while waiting on semaphore to read messages for ID = %d", i_module);
    }

    if (0 == ret)
    {
        ret = OS_mutex_lock(&msg_queue->mutex);

        if (0 != ret)
        {
            LOG_ERR("COM : error while locking mutex for queue %d", i_module);
        }
        else
        {
            /* Copy the message in the external buffer */
            memcpy(o_msg, &msg_queue->list[msg_queue->cur_index], sizeof(t_com_msg_struct));

            /* Update the index to the next unread message */
            msg_queue->cur_index = (msg_queue->cur_index + 1) % COM_MAX_NB_MSG;

            /* Reduce the number of messages in the queue */
            msg_queue->nb_msg = msg_queue->nb_msg - 1;

            /* Release the mutex */
            ret = OS_mutex_unlock(&msg_queue->mutex);
        }
    }

    return ret;
}

/* Register a module with its subscribing list */
int COM_msg_register(t_com_id_modules i_module, int *o_semfd)
{
    int ret = 0;
    t_com_msg_list *msg_queue;

    switch (i_module)
    {
        case COM_ID_MAIN:
        case COM_ID_OS:
        case COM_ID_COM:
        case COM_ID_CMD:
        case COM_ID_MODULE:
        case COM_ID_FAN:
        case COM_ID_TEMP:
        case COM_ID_REMOTE:
            break;
        case COM_ID_NULL:
        case COM_ID_NB:
        default:
            LOG_ERR("COM : wrong ID for module during subscription, module = %d", i_module);
            ret = -1;
            break;
    }

    if (0 == ret)
    {
        msg_queue = &com_list_queues[i_module];

        if (msg_queue->semfd.is_init == OS_RET_OK)
            *o_semfd = msg_queue->semfd.fd;
        else
            ret = -2;
    }

    return ret;
}

/*****************************************************************************/
/*                               Local functions                             */
/*****************************************************************************/

int com_init_msg(void)
{
    int ret = 0;
    int ii;

    /* Init all the mutexes for the queues */
    for (ii = 0; ii < COM_ID_NB; ii++)
    {
        com_list_queues[ii].cur_index = 0;
        com_list_queues[ii].nb_msg = 0;

        ret = OS_mutex_init(&com_list_queues[ii].mutex);

        if (0 != ret)
        {
            LOG_ERR("COM : error while initialising mutex for queue n°%d, ret = %d", ii, ret);
            com_list_queues[ii].is_init = OS_RET_KO;
            continue;
        }

        ret = OS_semfd_init(&com_list_queues[ii].semfd, 0);

        if (0 != ret)
        {
            LOG_ERR("COM : error while initialising semfd for queue n°%d, ret = %d", ii, ret);
            com_list_queues[ii].is_init = OS_RET_KO;
        }
        else
            com_list_queues[ii].is_init = OS_RET_OK;

        LOG_INF3("COM : init de la queue n°%d = %d", ii, ret);
    }

    return ret;
}

int com_stop_msg(void)
{
    int ret = 0;
    int ii;

    /* Destroy all the mutexes for the queues */
    for (ii = 0; ii < COM_BASE_LAST; ii++)
    {
        ret += OS_mutex_destroy(&com_list_queues[ii].mutex);

        if (ret)
           LOG_ERR("OS : error while destroying queue's mutex, index = %d", ii);

        ret += OS_semfd_destroy(&com_list_queues[ii].semfd);

        if (ret)
           LOG_ERR("OS : error while destroying queue's semfd, index = %d", ii);
    }

    return ret;
}
