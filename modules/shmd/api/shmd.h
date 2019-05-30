#pragma once

// Includes
#include "os.h"

// Types et structures
typedef struct
{
    int fan_speed;
    int temp_sys;
} shmd_sysdata_t;

// API pour module SHMD
int SHMD_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int SHMD_stop(void);

// Recuperation des pointeurs vers les donnees systemes
int SHMD_getPtrSystemData(shmd_sysdata_t **p_o_data);
int SHMD_givePtrSystemData(void);

