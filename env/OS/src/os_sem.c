/* API for thread handling */

/*********************************************************************/
/*                         Global includes                           */
/*********************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <poll.h>
#include <errno.h>

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

int OS_sem_init(OS_semaphore_t *i_sem, t_uint32 i_value)
{
    int ret = 0;

    if (!i_sem)
        ret = -1;

    if (0 == ret)
    {
        /* Check init status for the semaphore */
        if (i_sem->is_init != OS_RET_OK)
        {
            /* Semaphores are not to be shared among processes */
            ret = sem_init(&i_sem->semaphore, 0, i_value);

            if (0 == ret)
                i_sem->is_init = OS_RET_OK;
        }
    }

    if (0 == ret)
        i_sem->init_value = i_value;

    return ret;
}

int OS_sem_destroy(OS_semaphore_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
        ret = -1;

    if (0 == ret)
    {
        if (OS_RET_KO == i_sem->is_init)
            ret = 0;
        else
        {
            ret = sem_destroy(&i_sem->semaphore);
        }

        if (0 == ret)
            i_sem->is_init = OS_RET_KO;
    }

    return ret;
}

int OS_sem_post(OS_semaphore_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
            ret = sem_post(&i_sem->semaphore);
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -2;
        }
    }

    return ret;
}

int OS_sem_wait(OS_semaphore_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
            ret = sem_wait(&i_sem->semaphore);
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -2;
        }
    }

    return ret;
}

int OS_sem_trywait(OS_semaphore_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
            ret = sem_trywait(&i_sem->semaphore);
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -2;
        }

        if ( (-1 == ret) && (EAGAIN == errno) )
            ret = 1;
    }

    return ret;
}

/* Same function as OS_sem_init but with eventfd */
int OS_semfd_init(OS_semfd_t *i_sem, t_uint32 i_value)
{
    int ret = 0;

    if (!i_sem)
        ret = -1;

    if (0 == ret)
    {
        /* Check init status for the semaphore */
        if (i_sem->is_init != OS_RET_OK)
        {
            /* Declare the eventfd to be a semaphore */
            i_sem->fd = eventfd(i_value, EFD_SEMAPHORE);

            if (-1 == i_sem->fd)
            {
                LOG_ERR("OS : error while creating semaphore file descriptor, errno = %d", errno);
                i_sem->is_init = OS_RET_KO;
                ret = -2;
            }
            else
            {
                i_sem->is_init = OS_RET_OK;
                ret = 0;
            }
        }
    }

    if (0 == ret)
        i_sem->init_value = i_value;

    return ret;
}

int OS_semfd_destroy(OS_semfd_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
        ret = -1;

    if (0 == ret)
    {
        if (OS_RET_KO == i_sem->is_init)
            ret = 0;
        else
            ret = close(i_sem->fd);

        if (0 == ret)
        {
            i_sem->init_value = 0;
            i_sem->is_init = OS_RET_KO;
        }
        else
        {
            LOG_ERR("OS : error while closing semaphore file descriptor, errno = %d", errno);
            ret = -2;
        }
    }

    return ret;
}

int OS_semfd_post(OS_semfd_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
        {
            t_uint64 buf;
            buf = 1;

            ret = write(i_sem->fd, &buf, sizeof(buf));

            if (sizeof(buf) == ret)
               ret = 0;
            else
            {
               LOG_ERR("OS : error while posting to semaphore, ret = %d (expected %d)", ret, sizeof(buf));
               ret = -2;
            }
        }
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -4;
        }
    }

    return ret;
}

int OS_semfd_wait(OS_semfd_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
        {
            t_uint64 buf;
            buf = 1;

            ret = read(i_sem->fd, &buf, sizeof(buf));

            if (sizeof(buf) == ret)
               ret = 0;
            else
               ret = -2;
        }
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -4;
        }
    }

    return ret;
}

int OS_semfd_trywait(OS_semfd_t *i_sem)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
        {
            struct pollfd try_fd;

            try_fd.fd = i_sem->fd;
            try_fd.events = POLLIN;

            /* Check whether the file descriptor is ready for reading */
            ret = poll(&try_fd, 1, 0);

            /* Read only if the call would not block */
            if ( (1 == ret) && (POLLIN & try_fd.revents) )
            {
                t_uint64 buf;
                buf = 1;

                ret = read(i_sem->fd, &buf, sizeof(buf));

                if (sizeof(buf) == ret)
                    ret = 0;
                else
                    ret = -2;
            }
            else
            {
                /* Call would block */
                ret = 1;
            }
        }
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -4;
        }
    }

    return ret;
}

int OS_semfd_timedwait(OS_semfd_t *i_sem, int i_timeout)
{
    int ret = 0;

    if (NULL == i_sem)
    {
        LOG_ERR("OS : null pointer to semaphore");
        ret = -1;
    }

    if (0 == ret)
    {
        if (OS_RET_OK == i_sem->is_init)
        {
            struct pollfd try_fd;

            try_fd.fd = i_sem->fd;
            try_fd.events = POLLIN;

            /* Check whether the file descriptor is ready for reading */
            ret = poll(&try_fd, 1, i_timeout);

            /* Read only if the call would not block */
            if ( (1 == ret) && (POLLIN & try_fd.revents) )
            {
                t_uint64 buf;
                buf = 1;

                ret = read(i_sem->fd, &buf, sizeof(buf));

                if (sizeof(buf) == ret)
                    ret = 0;
                else
                    ret = -2;
            }
            else
            {
                /* Call would block */
                ret = 1;
            }
        }
        else
        {
            LOG_ERR("OS : semaphore not initialised");
            ret = -4;
        }
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

