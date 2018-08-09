#pragma once

#include <stdio.h>

#include "base.h"
// Defines
#define COM_UNIX_PATH_MAX   108

// Typedef
typedef struct
{
    t_uint32 addr;
    uint16_t port;
} t_com_inet_data;


// Fonctions API
int COM_create_socket(int i_family, int i_type, int i_proto, char *i_data);
