#pragma once

#include <pthread.h>

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*loop_func)(void *);

typedef struct
{
    pthread_t thread;
    pthread_attr_t attr;
    loop_func loop;
} OS_thread_t;

int OS_create_timer(void);

int OS_create_thread(OS_thread_t * p_o_thread,
                     void * args);

int OS_joint_thread(OS_thread_t * p_i_thread, void **retval);

int OS_detach_thread(OS_thread_t * p_i_thread);

#ifdef __cplusplus
}
#endif

