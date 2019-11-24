/* API for thread handling */

/*********************************************************************/
/*                         Global includes                           */
/*********************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <signal.h>

/*********************************************************************/
/*                          Local includes                           */
/*********************************************************************/

#include "base.h"
#include "integ_log.h"
#include "os.h"

/*********************************************************************/
/*                         Global variables                          */
/*********************************************************************/

/*********************************************************************/
/*                         API functions                             */
/*********************************************************************/

/* Creation of a thread without any options nor arguments */
int OS_create_thread(OS_thread_t *p_o_thread,
                     void *args)
{
   int ret = 0;

   /* Creation of the thread */
   ret = pthread_create(&(p_o_thread->thread), NULL, p_o_thread->loop, args);

   if (ret != 0)
      LOG_ERR("OS : Error while creating thread (code = %d)", ret);

   return ret;
}

/* Allow the caller to join a thread for it to end correctly */
int OS_joint_thread(OS_thread_t * i_p_thread, void **retval)
{
   int ret = 0;

   /* Joining thread here */
   ret = pthread_join(i_p_thread->thread, retval);

   if (ret != 0)
      LOG_ERR("OS : Error joining thread (code = %d)", ret);

   return ret;
}

/* Allow to detach thread in argument and run it as a daemon */
int OS_detach_thread(OS_thread_t * i_p_thread)
{
   int ret = 0;

   /* Detaching thread here */
   ret = pthread_detach(i_p_thread->thread);

   if (ret != 0)
      LOG_ERR("OS : Error detaching thread (code = %d)", ret);

   return ret;
}

/* Initialisation of a mutex */
int OS_mutex_init(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : bad pointer to mutex");
        ret = -1;
    }
    else if (OS_RET_OK == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex already initialised");
        ret = -2;
    }
    else
    {
        ret = pthread_mutex_init(&(i_mutex->mutex), &(i_mutex->attr));

        if (0 == ret)
        {
            i_mutex->is_init = OS_RET_OK;
        }
    }

    return ret;
}

/* Destruction of a mutex */
int OS_mutex_destroy(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : bad pointer to mutex");
        ret = -1;
    }
    else if (OS_RET_KO == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex already destroyed or not initialised");
        ret = -2;
    }
    else
    {
        ret = pthread_mutex_destroy(&(i_mutex->mutex));

        if (0 == ret)
        {
            i_mutex->is_init = OS_RET_KO;
        }
    }

    return ret;
}

/* Lock mutex in argument if already initialised */
int OS_mutex_lock(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : bad pointer to mutex");
        ret = -1;
    }
    else if (OS_RET_KO == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex not initialised");
        ret = -2;
    }
    else
    {
        ret = pthread_mutex_lock(&(i_mutex->mutex));
    }

    return ret;
}

/* Unlock the mutex in argument if already initialised */
int OS_mutex_unlock(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : bad pointer to mutex");
        ret = -1;
    }
    else if (OS_RET_KO == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex not initialised");
        ret = -2;
    }
    else
    {
        ret = pthread_mutex_unlock(&(i_mutex->mutex));
    }

    return ret;
}

/* Send <i_signal> to the <i_p_thread> in argument */
int OS_signal_send(OS_thread_t *i_p_thread, int i_signal)
{
    int ret = 0;

    if (!i_p_thread)
    {
        LOG_ERR("OS : wrong pointer to thread");
        ret = -1;
    }

    if (0 == ret)
    {
        /* Send a signal to the thread */
        ret = pthread_kill(i_p_thread->thread, i_signal);

        if (0 != ret)
        {
            LOG_ERR("OS : error while sending signal to a thread, ret = %d", ret);
            ret = -2;
        }
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

