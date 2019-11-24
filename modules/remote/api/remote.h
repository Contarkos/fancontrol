#pragma once

/* Includes */
#include "os.h"

#define NB_INSTANCES_REMOTE     1
#define REMOTE_SOCKET_NAME      "/tmp/remote_socket"

#define REMOTE_MULTICAST_ADDR   "239.0.2.4"
#define REMOTE_MULTICAST_PORT   31001

/* Types */
typedef struct
{
    t_uint32 fan_rpm;
    t_uint32 temp_temp;
} __attribute__((packed)) t_remote_status;

/* API pour module REMOTE */
int REMOTE_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int REMOTE_stop(void);

