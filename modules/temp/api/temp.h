#ifndef TEMP_H_
#define TEMP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Defines */
#define NB_INSTANCES_TEMP       1
#define TEMP_SOCKET_NAME        "/tmp/temp_sock"

/* Typedef */
typedef enum
{
    TEMP_VALIDITY_INVALID = 0,
    TEMP_VALIDITY_VALID = 1
} t_temp_validity;

typedef struct
{
    float fan_temp;
    float room_temp;
    t_temp_validity fan_temp_valid;
    t_temp_validity room_temp_valid;
} __attribute__((packed)) t_temp_data;

typedef struct
{
    t_uint32 tic;
} __attribute__((packed)) t_temp_tic;

/* API for TEMP module */
int TEMP_start(OS_mutex_t *m_main, OS_mutex_t *m_mod);

int TEMP_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* TEMP_H_ */
