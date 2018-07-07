// Gestion des threads de manière globale

// Includes globaux
#include <stdio.h>
#include <pthread.h>

// Includes locaux
#include "os.h"


int OS_create_thread(OS_thread_t *p_o_thread,
                     void *args)
{
   int ret = 0;

   // Creation du thread
   ret = pthread_create(&(p_o_thread->thread), NULL, p_o_thread->loop, args);

   if (ret != 0)
   {
      printf("OS : Erreur pendant la création d'un thread (code = %d) \n", ret);
   }

   return ret;
}

int OS_joint_thread(OS_thread_t * p_i_thread, void **retval)
{
   int ret = 0;

   // Creation du thread
   ret = pthread_join(p_i_thread->thread, retval);

   if (ret != 0)
   {
      printf("OS : Erreur pendant la jonction d'un thread (code = %d) \n", ret);
   }

   return ret;
}

int OS_detach_thread(OS_thread_t * p_i_thread)
{
   int ret = 0;

   // Creation du thread
   ret = pthread_detach(p_i_thread->thread);

   if (ret != 0)
   {
      printf("OS : Erreur pendant la jonction d'un thread (code = %d) \n", ret);
   }

   return ret;
}


