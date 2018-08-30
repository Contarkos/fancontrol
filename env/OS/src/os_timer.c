// Includes globaux
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

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

#define OS_USEC2NSEC        (1000)
#define MAX_TIMER_COUNT     (1000)
#define OS_TIMER_TIMEOUT    (100)

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

typedef struct timer_node
{
    int fd;
    timer_func callback;
    void * data;
    unsigned int usec;
    t_os_timer_type type;
    struct timer_node * next;
} t_os_timer_node;

static void * os_timer_thread(void * data);
static t_os_timer_node * _get_timer_from_fd(int fd);
static pthread_t os_timer_thread_id;
static t_os_timer_node *os_timer_head = NULL;

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Creation d'un timer
size_t OS_create_timer(t_uint32 i_usec, timer_func i_handler, t_os_timer_type i_type, void * i_data)
{
    t_os_timer_node * n = NULL;

    n = (t_os_timer_node *) malloc(sizeof(t_os_timer_node));

    if (NULL == n)
    {
        LOG_ERR("OS : erreur allocation timer");
    }
    else
    {
        n->callback = i_handler;
        n->data = i_data;
        n->usec = i_usec;
        n->type = i_type;

        n->fd = timerfd_create(CLOCK_REALTIME, 0);

        // Si le timer n'a pas pu être crée on sort en libérant la mémoire du malloc
        if (-1 == n->fd)
        {
            // Libération de la mémoire avant exit
            free(n);
        }
        else
        {
            // Insertion du timer dans la liste
            n->next = os_timer_head;
            os_timer_head = n;
        }
    }

    return (size_t)n;
}

// L'ID est le file descriptor donc on peut caster directement la structure (bof)
int OS_start_timer(size_t i_timer_id)
{
    int ret = 0;
    t_os_timer_node *n = (t_os_timer_node *)i_timer_id;
    struct itimerspec t;

    // On vérifie que le timer existe bien
    if (n)
    {
        // Reglage du temps au bout duquel le timer déclenche
        t.it_value.tv_sec = 0;
        t.it_value.tv_nsec = (long) (n->usec * OS_USEC2NSEC);

        // Reglage de la périodicité
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

int OS_stop_timer(size_t i_timer_id)
{
    int ret = 0;
    t_os_timer_node *tmp = NULL, *n = (t_os_timer_node *) i_timer_id;

    if (n)
    {
        close(n->fd);

        // Si on est sur le timer de tete dans la liste
        if (n == os_timer_head)
        {
            os_timer_head = os_timer_head->next;
        }
        else
        {
            // On parcourt la liste pour trouver n
            tmp = os_timer_head;

            while (tmp)
            {
                // On a trouvé n et il vaut tmp->next
                if (tmp->next == n)
                {
                    tmp->next = tmp->next->next;
                    break;
                }

                // On passe au suivant
                tmp = tmp->next;
            }

        }

        // Liberation de la mémoire si nécessaire
        if (n)
        {
            free(n);
        }
    }
    else
    {
        LOG_ERR("OS : Arret timer, timer inexistant (timer id = %d)", i_timer_id);
        ret = -1;
    }

    return ret;
}

// Fonction basique d'attente
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

    // Creation du timer qui lancera les timers
    ret = pthread_create(&os_timer_thread_id, NULL, os_timer_thread, NULL);

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
    int ret = 0;

    // Arret des timers
    while(os_timer_head) 
    {
        OS_stop_timer((size_t) os_timer_head);
    }

    // Arret du thread et suppression
    pthread_cancel(os_timer_thread_id);
    pthread_join(os_timer_thread_id, NULL);

    return ret;
}

/*********************************************************************/
/*                       Fonctions statiques                         */
/*********************************************************************/

// Récupération d'un noeud à partir du fd de son timer
static t_os_timer_node * _get_timer_from_fd(int fd)
{
    t_os_timer_node * tmp = os_timer_head;

    while(tmp)
    {
        if(tmp->fd == fd) break;

        tmp = tmp->next;
    }

    return tmp;
}

static void * os_timer_thread(void * data)
{
    struct pollfd ufds[MAX_TIMER_COUNT] = {{0}};
    int read_fds = 0, ss;
    t_os_timer_node *tmp = NULL;
    t_uint32 ii, max = 0;
    uint64_t exp;

    UNUSED_PARAMS(data);

    while(1)
    {
        // On active l'annulabilité du thread pour pouvoir l'arreter
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();

        // On desactive l'annulabilité du thread pour être sur d'avoir le temps de tout faire proprement
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        max = 0;
        tmp = os_timer_head;

        // Reinit de la zone mémoire
        memset(ufds, 0, sizeof(struct pollfd) * MAX_TIMER_COUNT);

        while(tmp)
        {
            ufds[max].fd = tmp->fd;
            ufds[max].events = POLLIN;
            max++;

            tmp = tmp->next;
        }
        read_fds = poll(ufds, max, OS_TIMER_TIMEOUT);

        // Si rien ne s'est passé on continue
        if (read_fds <= 0) continue;

        for (ii = 0; ii < max; ii++)
        {
            if (ufds[ii].revents & POLLIN)
            {
                ss = read(ufds[ii].fd, &exp, sizeof(uint64_t));

                if (ss != sizeof(uint64_t)) continue;

                tmp = _get_timer_from_fd(ufds[ii].fd);

                // Pour ne pas lancer les callbacks qui n'existent plus
                if(tmp && tmp->callback) 
                {
                    tmp->callback((int)tmp, tmp->data);
                }
            }
        }
    }

    return NULL;
}
