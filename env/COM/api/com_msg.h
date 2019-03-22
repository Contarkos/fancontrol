#pragma once

/*********************************************************************/
/*               Defines pour les ID des messages                    */
/*********************************************************************/

// Defines des offset
#define COM_NB_MSG_BASE     255
#define COM_BASE_MAIN       0
#define COM_BASE_OS         1
#define COM_BASE_COM        2
#define COM_BASE_CMD        3
#define COM_BASE_MODULE     4
#define COM_BASE_FAN        5
#define COM_BASE_TEMP       6
#define COM_BASE_REMOTE     7
#define COM_BASE_LAST       8

// Messages MAIN
#define MAIN_SHUTDOWN       (0 + (COM_BASE_MAIN * COM_NB_MSG_BASE))

// Messages FAN
#define FAN_MODE            (0 + (COM_BASE_FAN * COM_NB_MSG_BASE))
#define FAN_POWER           (1 + (COM_BASE_FAN * COM_NB_MSG_BASE))
#define FAN_TIMER           (2 + (COM_BASE_FAN * COM_NB_MSG_BASE))

// Messages TEMP
#define TEMP_DATA           (0 + (COM_BASE_TEMP * COM_NB_MSG_BASE))
#define TEMP_TIMER          (1 + (COM_BASE_TEMP * COM_NB_MSG_BASE))

// Messages REMOTE
#define REMOTE_TIMER        (0 + (COM_BASE_REMOTE * COM_NB_MSG_BASE))

// Dernier message
#define COM_TOTAL_MSG       (0 + (COM_BASE_LAST * COM_NB_MSG_BASE))

/*********************************************************************/
/*                           Typedef                                 */
/*********************************************************************/
