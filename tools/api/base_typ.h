#pragma once

// Définition des types pour l'ensemble du soft
typedef int (*start_func)(std::mutex *p_i_mutex);
typedef int (*stop_func)(void);

typedef struct
{
    start_func mod_start;
    stop_func mod_stop;
} mod_type;
