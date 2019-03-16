#pragma once

// Defines
#define NB_INSTANCES_FAN        1
#define FAN_MAX_SPEED           1350
#define FAN_SOCKET_NAME         "/tmp/fan_sock"

// Types
typedef enum
{
    FAN_MODE_AUTO = 0,
    FAN_MODE_TEMP = 1,
    FAN_MODE_RPM = 2
} fan_e_mode;

typedef enum
{
    FAN_POWER_MODE_OFF = 0,
    FAN_POWER_MODE_ON = 1
} fan_e_power_mode;

typedef struct
{
    fan_e_mode mode;
} __attribute__((packed)) t_fan_mode;

typedef struct
{
    fan_e_power_mode power_mode;
} __attribute__((packed)) t_fan_power_mode;

// Variables globales

// API pour module FAN
int FAN_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int FAN_stop(void);
