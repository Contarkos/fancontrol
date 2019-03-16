#pragma once

#define NB_INSTANCES_REMOTE     1
#define REMOTE_SOCKET_NAME      "/tmp/remote_socket"

// API pour module REMOTE
int REMOTE_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int REMOTE_stop(void);

