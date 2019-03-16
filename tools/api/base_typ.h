#pragma once

// Includes globaux
#include <mutex>

// Includes locaux
#ifdef __cplusplus
extern "C"
{
#endif

#include "os.h"

#ifdef __cplusplus
}
#endif

// Définition des types pour l'ensemble du soft
typedef int (*start_func)(OS_mutex_t *p_i_main_mutex, OS_mutex_t *p_i_mod_mutex);
typedef int (*stop_func)(void);

typedef struct
{
    start_func mod_start;
    stop_func mod_stop;
    OS_mutex_t mutex_mod;
    OS_mutex_t mutex_main;
} mod_type;
