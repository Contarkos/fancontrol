#ifndef BASE_H_
#define BASE_H_

/* Définition de types */
typedef char t_int8;
typedef unsigned char t_uint8;
typedef short t_int16;
typedef unsigned short t_uint16;
typedef int t_int32;
typedef unsigned int t_uint32;
typedef volatile int v_int32;
typedef volatile unsigned int v_uint32;
typedef long long int t_int64;
typedef unsigned long long int t_uint64;

/* Fichier contenant les macros transverses */

#define UNUSED_PARAMS(x)        ((void) x)
#define BASE_MAX(a, b)          (a > b ? a : b)
#define BASE_MIN(a, b)          (a < b ? a : b)
#define BASE_BORNE(a, min, max) (a > min ? (a < max ? a : max) : min)

#define MAX_UINT_16         65535

#define BASE_OK             0

#endif /* BASE_H_ */

