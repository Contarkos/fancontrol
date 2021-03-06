#ifndef INTEG_LOG_H_
#define INTEG_LOG_H_

/*****************************************************************************/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                                                                           */
/* The main value is defined in the Makefile and can be manually overriden   */
/* in each file by defining INTEGRATION_LOG_LEVEL before including this      */
/* header.                                                                   */
/*                                                                           */
/*****************************************************************************/

/* We use for INTEGRATION_LOG_LEVEL the same values as in the "LOG" module: */
/*   * LOG_LVL_SYSERR = 0 // System error */
/*   * LOG_LVL_SYSWNG = 1 // System warning */
/*   * LOG_LVL_APPERR = 2 // Application error */
/*   * LOG_LVL_APPWNG = 3 // Application warning */
/*   * LOG_LVL_INF1   = 4 // 1st information level (useful for validation tests) */
/*   * LOG_LVL_INF2   = 5 // 2nd information level (useful for integration tests) */
/*   * LOG_LVL_INF3   = 6 // 3rd information level (useful for unit tests) */

#if defined(INTEGRATION_LOG_LEVEL)
    #include <stdio.h>
#endif /*INTEGRATION_LOG_LEVEL */

/* ================ */
/*  LOG_LVL_APPERR */
/* ================ */
#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 2)
    #include <errno.h>  /* errno */
    #define LOG_ERR(...)                                                   \
    do {                                                                               \
        (void)printf("### ERR  ### - %-25s line %4d: ", __FILE__ , __LINE__);          \
        (void)printf(__VA_ARGS__); (void)printf("\n"); (void)fflush(stdout);           \
    } while (0)
    #define LOG_ERR_CODE(code, ...)                                         \
    do {                                                                               \
        (void)printf("### ERR  ### - %-25s line %4d: ", __FILE__ , __LINE__);          \
        (void)printf("%s(%d), ", strerror(code), code);                                \
        (void)printf(__VA_ARGS__); (void)printf("\n"); (void)fflush(stdout);           \
    } while (0)
#else
    #define LOG_ERR(...)                do { ; } while(0)
    #define LOG_ERR_CODE(code, ...)     do { ; } while(0)
#endif

/* ================ */
/*  LOG_LVL_APPWNG */
/* ================ */
#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 3)
    #define LOG_WNG(...)                                                   \
    do {                                                                               \
        (void)printf("### WNG  ### - %-25s line %4d: ", __FILE__ , __LINE__);          \
        (void)printf(__VA_ARGS__); (void)printf("\n"); (void)fflush(stdout);           \
    } while (0)
#else
    #define LOG_WNG(...)                do { ; } while(0)
#endif

/* ================ */
/*  LOG_LVL_INF1 */
/* ================ */
#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 4)
    #define LOG_INF1(...)                                                  \
    do {                                                                               \
        (void)printf("### INF1 ### - %-25s line %4d: ", __FILE__ , __LINE__);          \
        (void)printf(__VA_ARGS__); (void)printf("\n"); (void)fflush(stdout);           \
    } while (0)
#else
    #define LOG_INF1(...)               do { ; } while(0)
#endif

/* ================ */
/*  LOG_LVL_INF2 */
/* ================ */
#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 5)
    #define LOG_INF2(...)                                                  \
    do {                                                                               \
        (void)printf("### INF2 ### - %-25s line %4d: ", __FILE__ , __LINE__);          \
        (void)printf(__VA_ARGS__); (void)printf("\n"); (void)fflush(stdout);           \
    } while (0)
#else
    #define LOG_INF2(...)               do { ; } while(0)
#endif

/* ================ */
/*  LOG_LVL_INF3 */
/* ================ */
#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 6)
    #define LOG_INF3(...)                                                  \
    do {                                                                               \
        (void)printf("### INF3 ### - %-25s line %4d: ", __FILE__ , __LINE__);          \
        (void)printf(__VA_ARGS__); (void)printf("\n"); (void)fflush(stdout);           \
    } while (0)
#else
    #define LOG_INF3(...)               do { ; } while(0)
#endif

#endif /* INTEG_LOG_H_ */

