/* Global includes */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

/* Local includes */
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

/* Demande d'ouverture d'un fichier d'interruption */
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

/* Fermeture du fichier d'interruption */
int OS_irq_close(int i_fd)
{
    int ret = 0;

    if (0 == i_fd)
    {
        LOG_ERR("OS : file descriptor IRQ invalide, i_fd = %d", i_fd);
        ret = -1;
    }
    else
    {
        if (0 != close(i_fd))
        {
            LOG_WNG("OS : probleme lors de la fermeture fichier IRQ, i_fd = %d", i_fd);
            ret = -2;
        }
    }

    return ret;
}
