// Global includes
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Local includes
#include "integ_log.h"
#include "com.h"
#include "com_msg.h"
#include "com_socket.h"
#include "com_message.h"

// Variables globales
int com_extern_socket = -1;

int COM_init(void)
{
    int ret = 0;
    struct sockaddr_in d;
    d.sin_family = AF_INET;
    d.sin_port = htons(21001);
    d.sin_addr.s_addr = INADDR_ANY;

    /* Init the messages structures */
    ret = com_init_msg();

    if (0 == ret)
    {
        /* Open the external socket */
        com_extern_socket = COM_create_socket(AF_INET, SOCK_STREAM, 0, (char *) &d, sizeof(d));

        if (-1 == com_extern_socket)
        {
            LOG_ERR("COM : erreur création socket externe\n");
            ret = -1;
        }
        else
        {
            /* Listen on the right port */
            ret = COM_socket_listen(com_extern_socket, COM_EXTERN_BACKLOG);
        }
    }

    return ret;
}

int COM_stop(void)
{
    int ret = 0;

    ret = COM_close_socket(com_extern_socket);

    if (0 != ret)
        LOG_ERR("COM : error while closing external socket, ret = %d", ret);

    ret = com_stop_msg();

    if (0 != ret)
        LOG_ERR("COM : error while stopping messages instances, ret = %d", ret);

    return ret;
}
