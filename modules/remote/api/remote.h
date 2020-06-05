#ifndef REMOTE_H_
#define REMOTE_H_

/* Includes */
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NB_INSTANCES_REMOTE     1
#define REMOTE_SOCKET_NAME      "/tmp/remote_socket"

#define REMOTE_MULTICAST_ADDR   "239.0.2.4"
#define REMOTE_MULTICAST_PORT   31001

/* Types */
typedef struct
{
    t_uint32 fan_rpm;
    t_int32 temp_temp;
    t_uint32 temp_valid;
} __attribute__((packed)) t_remote_status;

/* API for REMOTE module */
int REMOTE_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int REMOTE_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* REMOTE_H_ */

