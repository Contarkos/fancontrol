#pragma once

// Includes
#include <mutex>

// Définition des types pour l'ensemble du soft
typedef int (*start_func)(void);
typedef int (*stop_func)(void);

typedef struct
{
    std::mutex mod_mutex;
    start_func mod_start;
    stop_func mod_stop;
} mod_type;
