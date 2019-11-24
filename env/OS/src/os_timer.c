/* Includes globaux */
#include <signal.h>
#include <time.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

/* Includes locaux */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

#define OS_USEC2NSEC        (1000)
#define MAX_TIMER_COUNT     (10)
#define OS_TIMER_TIMEOUT    (100)

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

typedef struct timer_node
{
    int fd;
    int index;
    timer_func callback;
    void * data;
    unsigned int usec;
    t_os_timer_type type;
} t_os_timer_node;

/*********************************************************************/
/*                       Déclarations statiques                      */
/*********************************************************************/

static void * _os_timer_thread(void * data);
static t_os_timer_node * _get_timer_from_fd(int fd);

static int _os_get_free_index(t_os_timer_node *i_array);

static pthread_t os_timer_thread_id;
static t_os_timer_node timer_array[MAX_TIMER_COUNT] = { {0} };

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

/* Creation d'un timer */
int OS_create_timer(t_uint32 i_usec, timer_func i_handler, t_os_timer_type i_type, void * i_data)
{
    int ii = -1;

    /* Pas d'allocation dynamique */
    ii = _os_get_free_index(timer_array);

    if (-1 == ii)
    {
        LOG_ERR("OS : erreur allocation timer");
    }
    else
    {
        timer_array[ii].callback = i_handler;
        timer_array[ii].data = i_data;
        timer_array[ii].usec = i_usec;
        timer_array[ii].type = i_type;

        /* Tentative de création du timer */
        timer_array[ii].fd = timerfd_create(CLOCK_REALTIME, 0);

        /* Si le timer n'a pas pu être crée on sort en resettant la mémoire */
        if (-1 == timer_array[ii].fd)
        {
            /* Libération de la mémoire avant exit */
            memset(&(timer_array[ii]), 0, sizeof(t_os_timer_node));

            /* Pas de quoi faire un timer */
            ii = -2;
        }
        else
        {
            /* Sauvegarde de l'index */
            timer_array[ii].index = ii;
        }
    }

    return ii;
}

/* L'ID est l'index dans le tableau des timers donc on peut récupérer l'adresse */
int OS_start_timer(int i_timer_id)
{
    int ret = 0;
    t_os_timer_node *n = &(timer_array[i_timer_id]);
    struct itimerspec t;

    /* On vérifie que le timer existe bien */
    if (n->fd)
    {
        /* Reglage du temps au bout duquel le timer déclenche */
        t.it_value.tv_sec = 0;
        t.it_value.tv_nsec = (long) (n->usec * OS_USEC2NSEC);

        /* Reglage de la périodicité */
        t.it_interval.tv_sec = 0;

        switch (n->type)
        {
            case OS_TIMER_SINGLE_SHOT:
                t.it_interval.tv_nsec = 0;
                break;
            case OS_TIMER_PERIODIC:
                t.it_interval.tv_nsec = (long) (n->usec * OS_USEC2NSEC);
                break;
            default:
                LOG_ERR("OS : erreur de type pour le timer");
                ret = -2;
        }

        if (0 == ret)
        {
            timerfd_settime(n->fd, 0, &t, NULL);
        }
    }
    else
    {
        LOG_ERR("OS : Demarrage timer, timer inexistant (timer id = %d)", i_timer_id);
        ret = -1;
    }

    return ret;
}

int OS_stop_timer(int i_timer_id)
{
    int ret = 0;
    t_os_timer_node *n = &(timer_array[i_timer_id]);

    if (n->fd)
    {
        /* Fermeture du fichier */
        close(n->fd);
    }

    /* Clean de la zone mémoire */
    memset(n, 0, sizeof(t_os_timer_node));

    return ret;
}

/* Fonction basique d'attente */
void OS_usleep(int i_usec)
{
    struct timespec timer_enbl = {.tv_sec = 0, .tv_nsec = i_usec * OS_USEC2NSEC};
    nanosleep(&timer_enbl, NULL);
}

/*********************************************************************/
/*                       Fonctions internes                          */
/*********************************************************************/

int os_init_timer()
{
    int ret = 0;

    /* Init de la zone mémoire */
    memset(timer_array, 0, sizeof(t_os_timer_node) * MAX_TIMER_COUNT);

    /* Creation du timer qui lancera les timers */
    ret = pthread_create(&os_timer_thread_id, NULL, _os_timer_thread, NULL);

    if (0 != ret)
    {
        LOG_ERR("OS : pb création thread pour timer facility (code = %d)", ret);
    }
    else
    {
       LOG_INF1("OS : init TIMER ok");
    }

    return ret;
}

int os_end_timer()
{
    int ret = 0, ii;

    /* Arret des timers */
    for (ii = 0; ii < MAX_TIMER_COUNT; ii++)
    {
        ret += OS_stop_timer(ii);
    }

    /* Arret du thread et suppression */
    pthread_cancel(os_timer_thread_id);
    pthread_join(os_timer_thread_id, NULL);

    return ret;
}

/*********************************************************************/
/*                       Fonctions statiques                         */
/*********************************************************************/

/* Récupération d'un noeud à partir du fd de son timer */
static t_os_timer_node * _get_timer_from_fd(int fd)
{
    int ii;
    t_os_timer_node *tmp = NULL;

    for (ii = 0; ii < MAX_TIMER_COUNT; ii++)
    {
        if (timer_array[ii].fd == fd)
        {
            tmp = &(timer_array[ii]);
            break;
        }
    }

    return tmp;
}

static int _os_get_free_index(t_os_timer_node *i_array)
{
    int ret = -1, ii;

    for (ii = 0; ii < MAX_TIMER_COUNT; ii++)
    {
        if (0 == i_array[ii].fd)
        {
            ret = ii;
            break;
        }
    }

    return ret;
}

static void * _os_timer_thread(void * data)
{
    struct pollfd ufds[MAX_TIMER_COUNT] = {{0}};
    int read_fds = 0, ss;
    t_os_timer_node *tmp = NULL;
    t_uint32 jj, ii, max = 0;
    uint64_t exp;

    UNUSED_PARAMS(data);

    while(1)
    {
        /* On active l'annulabilité du thread pour pouvoir l'arreter */
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();

        /* On desactive l'annulabilité du thread pour être sur d'avoir le temps de tout faire proprement */
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        /* Reinit de la zone mémoire */
        memset(ufds, 0, sizeof(struct pollfd) * MAX_TIMER_COUNT);
        max = 0;

        /* On ne surveille que les timers initialisés */
        for (jj = 0; jj < MAX_TIMER_COUNT; jj++)
        {
            if (timer_array[jj].fd)
            {
                ufds[max].fd = timer_array[jj].fd;
                ufds[max].events = POLLIN;
                max++;
            }
        }

        /* Polling sur les file descriptors */
        read_fds = poll(ufds, max, OS_TIMER_TIMEOUT);

        /* Si rien ne s'est passé on continue */
        if (read_fds <= 0) continue;

        for (ii = 0; ii < max; ii++)
        {
            if (ufds[ii].revents & POLLIN)
            {
                ss = read(ufds[ii].fd, &exp, sizeof(uint64_t));

                if (ss != sizeof(uint64_t)) continue;

                tmp = _get_timer_from_fd(ufds[ii].fd);

                /* Pour ne pas lancer les callbacks qui n'existent plus */
                if(tmp && tmp->callback) 
                {
                    tmp->callback(tmp->index, tmp->data);
                }
            }
        }
    }

    return NULL;
}
