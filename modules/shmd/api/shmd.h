#ifndef SHMD_H_
#define SHMD_H_

/* Includes */
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Types et structures */
typedef struct
{
    int temp_room;
    t_uint32 temp_room_valid;
    int temp_sys;
    t_uint32 temp_sys_valid;
} shmd_tempdata_t;

typedef struct
{
    t_uint32 fan_speed;
    t_uint32 fan_target;
} shmd_fanstatus_t;

/* API pour module SHMD */
int SHMD_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int SHMD_stop(void);

/* Recuperation des pointeurs vers les donnees temperatures */
int SHMD_getPtrTempData(shmd_tempdata_t **p_o_data);
int SHMD_givePtrTempData(void);

/* Pointers to fan status */
int SHMD_getPtrFanStatus(shmd_fanstatus_t **p_o_data);
int SHMD_givePtrFanStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* SHMD_H_ */

