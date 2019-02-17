#pragma once

#define NB_INSTANCES_REMOTE     1
#define REMOTE_SOCKET_NAME      "/tmp/remote_socket"

// API pour module REMOTE
int REMOTE_start(std::mutex *m_main, std::mutex *m_mod);

int REMOTE_stop(void);

