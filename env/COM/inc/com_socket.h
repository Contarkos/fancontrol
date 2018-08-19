#pragma once

// Global includes
#include <stdio.h>

// Local includes
#include "com.h"

// Defines

// Typedef
typedef struct
{
    int list[COM_MAX_NB_MSG];
} t_com_msg_list;

// Variables globales
extern int com_extern_socket;
extern t_com_msg_list com_list_msg[COM_TOTAL_MSG];

// Prototypes
int com_bind_socket_unix(int fd, char *data);
int com_bind_socket_inet(int fd, char *data);

int com_connect_unix(int fd, char *data);
int com_connect_inet(int fd, char *data);

int com_add_fd_to_list(int i_fd, int i_id);
