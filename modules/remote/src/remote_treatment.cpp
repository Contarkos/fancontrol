// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "com.h"
#include "com_msg.h"
#include "remote_class.h"

/*********************************************************************/
/*                      Fonctions de classe                          */
/*********************************************************************/

void REMOTE::remote_timer_handler(int i_timer_id, void *i_data)
{
    REMOTE *p_this = reinterpret_cast<REMOTE *> (i_data);
    int dum;

    if (p_this && (p_this->timer_fd == i_timer_id))
    {
        COM_send_data(p_this->timeout_fd, REMOTE_TIMER, &dum, sizeof(dum), 0);
    }
}


int REMOTE::remote_treat_msg(int i_fd)
{
    int ret = 0, ss;
    t_com_msg m;

    if (0 > i_fd)
    {
        LOG_ERR("REMOTE : pas de file descriptor pour lecture message, fd = %d", i_fd);
        ret = -1;
    }
    else
    {
        // Recuperation des données du message
        ret = COM_receive_data(i_fd, &m, &ss);

        if (0 == ss)
        {
            LOG_WNG("REMOTE : mauvaise taille de message ");
            ret = -2;
        }
        else
        {
            switch (m.id)
            {
                case MAIN_SHUTDOWN:
                    ret = this->stop_and_exit();
                    break;
                case REMOTE_TIMER:
                    ret = this->remote_send_status();
                    break;
                default:
                    LOG_ERR("REMOTE : mauvaise ID pour message, id = %d", m.id);
                    ret = 1;
            }
        }
    }

    return ret;
}

// Traitement des messages UDP venant de l'exterieur
int REMOTE::remote_treat_udp(int i_fd)
{
    int ret = 0, ss;
    t_com_msg m;

    if (i_fd < 0)
    {
        LOG_ERR("REMOTE : wrong file descriptor for UDP socket, fd = %d", i_fd);
        ret = -1;
    }
    else
    {
        // Recuperation des données du message
        ret = COM_receive_data(i_fd, &m, &ss);

        if (0 == ss)
        {
            LOG_WNG("REMOTE : wrong data size");
            ret = -2;
        }
        else
        {
            switch (m.id)
            {
                default:
                    break;
            }
        }
    }

    return ret;
}