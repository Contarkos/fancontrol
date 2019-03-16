// Gestion des threads de manière globale

// Includes globaux
#include <stdio.h>
#include <pthread.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Creation d'un thread sans options
int OS_create_thread(OS_thread_t *p_o_thread,
                     void *args)
{
   int ret = 0;

   // Creation du thread
   ret = pthread_create(&(p_o_thread->thread), NULL, p_o_thread->loop, args);

   if (ret != 0)
   {
      LOG_ERR("OS : Erreur pendant la création d'un thread (code = %d)", ret);
   }

   return ret;
}

// Permet de joindre un thread pour qu'il se termine correctement
int OS_joint_thread(OS_thread_t * p_i_thread, void **retval)
{
   int ret = 0;

   // Creation du thread
   ret = pthread_join(p_i_thread->thread, retval);

   if (ret != 0)
   {
      LOG_ERR("OS : Erreur pendant la jonction d'un thread (code = %d)", ret);
   }

   return ret;
}

// Permet de lancer le thread en question en tant que daemon
int OS_detach_thread(OS_thread_t * p_i_thread)
{
   int ret = 0;

   // Creation du thread
   ret = pthread_detach(p_i_thread->thread);

   if (ret != 0)
   {
      LOG_ERR("OS : Erreur pendant le detachement d'un thread (code = %d)", ret);
   }

   return ret;
}

// Init d'un mutex
int OS_init_mutex(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : mauvais pointeur pour mutex");
        ret = -1;
    }
    else if (OS_RET_OK == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex deja initialise");
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

// Destruction d'un mutex
int OS_destroy_mutex(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : mauvais pointeur pour mutex");
        ret = -1;
    }
    else if (OS_RET_KO == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex deja detruit ou non initialise");
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

// Lock d'un mutex
int OS_lock_mutex(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : mauvais pointeur pour mutex");
        ret = -1;
    }
    else if (OS_RET_KO == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex non initialise");
        ret = -2;
    }
    else
    {
        ret = pthread_mutex_lock(&(i_mutex->mutex));
    }

    return ret;
}

// Unlock d'un mutex
int OS_unlock_mutex(OS_mutex_t *i_mutex)
{
    int ret = 0;

    if (NULL == i_mutex)
    {
        LOG_ERR("OS : mauvais pointeur pour mutex");
        ret = -1;
    }
    else if (OS_RET_KO == i_mutex->is_init)
    {
        LOG_WNG("OS : mutex non initialise");
        ret = -2;
    }
    else
    {
        ret = pthread_mutex_unlock(&(i_mutex->mutex));
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

