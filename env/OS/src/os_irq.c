/* Global includes */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

/*********************************************************************/
/*                         Global variables                          */
/*********************************************************************/


/*********************************************************************/
/*                         Functions API                             */
/*********************************************************************/

/* Request to open an IRQ file */
int OS_irq_request(const char *i_irq, int i_flags)
{
    int fd = 0;

    if (i_irq)
    {
        fd = open(i_irq, i_flags);

        if (-1 == fd)
            LOG_ERR("OS : error requesting IRQ, errno = %d", errno);
    }
    else
    {
        LOG_WNG("OS : incorrect name for IRQ");
    }

    return fd;
}

/* Closing IRQ file */
int OS_irq_close(int i_fd)
{
    int ret = 0;

    if (0 == i_fd)
    {
        LOG_ERR("OS : invalid IRQ file descriptor, i_fd = %d", i_fd);
        ret = -1;
    }
    else
    {
        if (0 != close(i_fd))
        {
            LOG_WNG("OS : error while closing IRQ file, i_fd = %d, errno = %d", i_fd, errno);
            ret = -2;
        }
    }

    return ret;
}
