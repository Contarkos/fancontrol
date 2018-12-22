// Global includes
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

// Local includes
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/


/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Demande d'ouverture d'un fichier d'interruption
int OS_irq_request(const char *i_irq, int i_flags)
{
    int fd = 0;

    if (i_irq)
    {
        fd = open(i_irq, i_flags);

        if (-1 == fd)
        {
            LOG_ERR("OS : erreur à la requête de l'interruption, errno = %d", errno);
        }
    }
    else
    {
        LOG_WNG("OS : nom incorrect pour l'interruption");
    }

    return fd;
}
