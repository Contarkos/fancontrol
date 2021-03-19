#ifndef BASE_H_
#define BASE_H_

/* Types definitions */
typedef char t_int8;
typedef unsigned char t_uint8;
typedef short t_int16;
typedef unsigned short t_uint16;
typedef int t_int32;
typedef unsigned int t_uint32;
typedef volatile int v_int32;
typedef volatile unsigned int v_uint32;
typedef float t_float32;
typedef double t_float64;
typedef long long int t_int64;
typedef unsigned long long int t_uint64;

/* Enum definitions */
typedef enum {
    BASE_FALSE = 0,
    BASE_TRUE = 1
} base_bool;

/* Fichier contenant les macros transverses */

#define UNUSED_PARAMS(x)        ((void) (x))
#define BASE_MAX(a, b)          ((a) > (b) ? (a) : (b))
#define BASE_MIN(a, b)          ((a) < (b) ? (a) : (b))
#define BASE_BORNE(a, min, max) ((a) > (min) ? ((a) < (max) ? (a) : (max)) : (min))

#define MAX_UINT_16         65535

#endif /* BASE_H_ */

