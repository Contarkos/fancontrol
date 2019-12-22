#pragma once

/*****************************************************************************/
/*                              Global includes                              */
/*****************************************************************************/

#include <stdio.h>

/*****************************************************************************/
/*                              Local includes                               */
/*****************************************************************************/

#include "com.h"
#include "com_msg.h"

/*****************************************************************************/
/*                                Defines                                    */
/*****************************************************************************/

/*****************************************************************************/
/*                                Typedef                                    */
/*****************************************************************************/

/*****************************************************************************/
/*                            Variables globales                             */
/*****************************************************************************/

extern int com_extern_socket;

/*****************************************************************************/
/*                              Prototypes                                   */
/*****************************************************************************/

int com_bind_socket_unix(int fd, char *data, size_t size_data);
int com_bind_socket_inet(int fd, char *data);

int com_connect_unix(int fd, char *data, size_t size_data);
int com_connect_inet(int fd, char *data);

int com_add_fd_to_list(int i_fd, int i_id);
