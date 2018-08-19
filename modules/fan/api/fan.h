#pragma once

// Defines
#define NB_INSTANCES_FAN        1
#define FAN_MAX_SPEED           1350
#define FAN_SOCKET_NAME         "/tmp/fan_socket"

// Types
typedef enum
{
    FAN_MODE_AUTO = 0,
    FAN_MODE_TEMP = 1,
    FAN_MODE_RPM = 2
} fan_e_mode;

// API pour module FAN
int FAN_start(std::mutex *m);

int FAN_stop(void);
