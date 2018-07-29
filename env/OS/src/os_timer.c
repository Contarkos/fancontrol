// Includes globaux
#include <signal.h>
#include <time.h>

// Includes locaux
#include "base.h"
#include "os.h"

#define OS_USEC2NSEC        (1000)

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Creation d'un timer
int OS_create_timer(void)
{
   int ret = 0;

   return ret;
}

int OS_start_timer(int i_timer_id)
{
   int ret = 0;

   UNUSED_PARAMS(i_timer_id);

   return ret;
}

void OS_usleep(int i_usec)
{
        struct timespec timer_enbl = {.tv_sec = 0, .tv_nsec = i_usec * OS_USEC2NSEC};
        nanosleep(&timer_enbl, NULL);
}
