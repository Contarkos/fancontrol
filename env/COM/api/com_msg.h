#pragma once

/*****************************************************************************/
/*                              Global includes                              */
/*****************************************************************************/

/*****************************************************************************/
/*                              Local includes                               */
/*****************************************************************************/

/*****************************************************************************/
/*                            Defines messages' ID                           */
/*****************************************************************************/

/* Defines of the offsets */
#define COM_NB_MSG_BASE     63

#define COM_BASE_MAIN       0
#define COM_BASE_OS         1
#define COM_BASE_COM        2
#define COM_BASE_CMD        3
#define COM_BASE_MODULE     4
#define COM_BASE_FAN        5
#define COM_BASE_TEMP       6
#define COM_BASE_REMOTE     7
#define COM_BASE_LAST       8

/* Messages MAIN */
#define MAIN_START          (0 + (COM_BASE_MAIN * COM_NB_MSG_BASE))
#define MAIN_SHUTDOWN       (1 + (COM_BASE_MAIN * COM_NB_MSG_BASE))

/* Messages FAN */
#define FAN_MODE            (0 + (COM_BASE_FAN * COM_NB_MSG_BASE))
#define FAN_POWER           (1 + (COM_BASE_FAN * COM_NB_MSG_BASE))
#define FAN_TIMER           (2 + (COM_BASE_FAN * COM_NB_MSG_BASE))

/* Messages TEMP */
#define TEMP_DATA           (0 + (COM_BASE_TEMP * COM_NB_MSG_BASE))
#define TEMP_TIMER          (1 + (COM_BASE_TEMP * COM_NB_MSG_BASE))

/* Messages REMOTE */
#define REMOTE_TIMER        (0 + (COM_BASE_REMOTE * COM_NB_MSG_BASE))
#define REMOTE_STATUS       (1 + (COM_BASE_REMOTE * COM_NB_MSG_BASE))

/* Dernier message */
#define COM_TOTAL_MSG       (0 + (COM_BASE_LAST * COM_NB_MSG_BASE))

/*****************************************************************************/
/*                               Typedef                                     */
/*****************************************************************************/

typedef enum e_com_id_modules
{
    COM_ID_NULL = 0,
    COM_ID_MAIN = 1,
    COM_ID_OS = 2,
    COM_ID_COM = 3,
    COM_ID_CMD = 4,
    COM_ID_MODULE = 5,
    COM_ID_FAN = 6,
    COM_ID_TEMP = 7,
    COM_ID_REMOTE = 8,
    COM_ID_NB
} t_com_id_modules;

