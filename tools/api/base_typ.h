#ifndef BASE_TYP_H_
#define BASE_TYP_H_

/* Global includes */

/* Local includes */
#ifdef __cplusplus
extern "C"
{
#endif

#include "os.h"

#ifdef __cplusplus
}
#endif

/* Global type definition */
typedef int (*start_func)(OS_mutex_t *p_i_main_mutex, OS_mutex_t *p_i_mod_mutex);
typedef int (*stop_func)(void);

typedef struct
{
    start_func mod_start;
    stop_func mod_stop;
    OS_mutex_t mutex_mod;
    OS_mutex_t mutex_main;
    t_uint32 msg_id;
} mod_type;

/* Proper return code */
#define BASE_E_OK           0
#define BASE_E_INVAL        1
#define BASE_E_IO           2
#define BASE_E_EXIST        3

#endif /* BASE_TYP_H_ */
