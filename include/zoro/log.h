/**
 * @file log.h
 * @copyright Copyright (c) 2024
 * @author Andrea Pepe <pepe.andmj@gmail.com>
 *
 * @brief Log utilities.
 *
 * Provide several utility functions to produce detailed logs.
 */
#pragma once
#ifndef __ZORO_LOG_H__

/*
 * To customize zorolog you may work on the following defines. Note that
 * the safe way to change any of the variable is by adding the relative
 * -D<var>=<value> arguments to your CFLAGS.
 */

#ifndef ZOROLOG_BACKTRACE_SIZE
    /**
     * @brief Size of log backtrace
     */
    #define ZOROLOG_BACKTRACE_SIZE 100
#endif

#ifndef ZOROLOG_DATE_CLOCK_TYPE
    /**
     * @brief Needs to be set to either CLOCK_REALTIME or 
     * CLOCK_REALTIME_COARSE (default). 
     * See <tt>man clock_gettime</tt> for more info.
     */
    #define ZOROLOG_DATE_CLOCK_TYPE CLOCK_REALTIME_COARSE
#endif 

#ifndef ZOROLOG_TIME_CLOCK_TYPE
    /**
     * @brief Needs to be set to either CLOCK_MONOTONIC_RAW or 
     * CLOCK_MONOTONIC_COARSE (default). 
     * See <tt>man clock_gettime</tt> for more info.
     */
    #define ZOROLOG_TIME_CLOCK_TYPE CLOCK_MONOTONIC_COARSE
#endif 

#ifndef ZOROLOG_TIME_SEC_BITS
    /**
     * @brief How many bits of the Epoch will be shown.
     * I.e. with 13 bits (default), the seconds will wrap every 8192 seconds
     */
    #define ZOROLOG_TIME_SEC_BITS 13
#endif

#ifndef ZOROLOG_TIME_SEC_DIGITS
    /**
     * @brief How many digits will be used to show seconds.
     * It depends on the `#ZOROLOG_TIME_SEC_BITS` macro: i.e. with 13 bits we
     * need 4 digits
     */
    #define ZOROLOG_TIME_SEC_DIGITS 4
#endif

#ifndef ZOROLOG_TIME_RESOLUTION
    /**
     * @brief Set this macro to \f(R\f) to have a time resolution of 
     * \f(10^{-R}\f) seconds.
     * I.e.: for microseconds, set it to 6.
     * Default is 3 (milliseconds).
     * Valid values are all integers between 0 and 9.
     */
    #define ZOROLOG_TIME_RESOLUTION 3
#endif

/* ======= Configuration end: do not make change below this line ======= */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <alloca.h>
#include <unistd.h>
#include <stdint.h>

#if defined(ZOROLOG_PRINT_TIME) || defined(ZOROLOG_PRINT_DATE)
/* Note that to use this function you must link with -lrt */
#include <time.h>
#if _POSIX_C_SOURCE < 199309L
#error "_POSIX_C_SOURCE too old to build with ZOROLOG_PRINT_TIME/DATE defined"
#endif
#endif

#ifndef zoro_fprintf
#define zoro_fprintf fprintf
#endif

#include <execinfo.h>

/*
 * static function are not showed by backtrace function, so you may set your
 * static functions as __zoro_static__ to have more info while debugging
 */
#if !defined(NDEBUG) || defined(ZOROLOG_NOSTATIC)
#define __zoro_static__
#else
#define __zoro_static__ static
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ZOROLOG_DUP_STDOUT	0x1
#define ZOROLOG_DUP_STDERR	0x2

#define ZOROLOG_APPEND		0x1

/**
 * @fn int zorolog_duplicate(const char *logfile, uint8_t stds, int flags)
 * @brief Duplicate the standard output and/or the standard error, copying them
 *        to a log file. What is written to the standards is actually 
 *        maintained; only a copy is written to the log file.
 *
 * @param logfile The filepath of the log file
 * @param stds Flag to specify standards to duplicate; use ZOROLOG_DUP_STDOUT
 *             and/or ZOROLOG_DUP_STDERR
 * @param flags Writing type flag (append or overwrite)
 *
 * @return -1 if some error occurred; 0 otherwise.
 */
int zorolog_duplicate(const char *logfile, uint8_t stds, int flags);

/**
 * @brief Print backtrace to standard error
 */
#define epylog_print_backtrace() do {                           \
    int __ret;                                                  \
    void *__bt_buff[ZOROLOG_BACKTRACE_SIZE];                    \
    __ret = backtrace(__bt_buff, ZOROLOG_BACKTRACE_SIZE);       \
    if (__ret == 0)                                             \
        (void)!write(STDERR_FILENO, "backtrace empty\n", 16);   \
    else if (__ret < 0)                                         \
        (void)!write(STDERR_FILENO, "backtrace failed\n", 17);  \
    else                                                        \
        (void)!write(STDERR_FILENO, "backtrace:\n", 11);        \
    backtrace_symbols_fd(__bt_buff, __ret, STDERR_FILENO);      \
} while (0)

#if ZOROLOG_TIME_RESOLUTION==0
#undef ZOROLOG_TIME_NSEC_DIV
#elif ZOROLOG_TIME_RESOLUTION==1
#define ZOROLOG_TIME_NSEC_DIV 100000000UL
#elif ZOROLOG_TIME_RESOLUTION==2
#define ZOROLOG_TIME_NSEC_DIV 10000000UL
#elif ZOROLOG_TIME_RESOLUTION==3
#define ZOROLOG_TIME_NSEC_DIV 1000000UL
#elif ZOROLOG_TIME_RESOLUTION==4
#define ZOROLOG_TIME_NSEC_DIV 100000UL
#elif ZOROLOG_TIME_RESOLUTION==5
#define ZOROLOG_TIME_NSEC_DIV 10000UL
#elif ZOROLOG_TIME_RESOLUTION==6
#define ZOROLOG_TIME_NSEC_DIV 1000UL
#elif ZOROLOG_TIME_RESOLUTION==7
#define ZOROLOG_TIME_NSEC_DIV 100UL
#elif ZOROLOG_TIME_RESOLUTION==8
#define ZOROLOG_TIME_NSEC_DIV 10UL
#elif ZOROLOG_TIME_RESOLUTION==9
#define ZOROLOG_TIME_NSEC_DIV 1UL
#else
#error "Invalid value for ZOROLOG_TIME_RESOLUTION"
#endif


#ifdef ZOROLOG_PRINT_TIME

#define ZOROLOG_TD_INIT \
    struct timespec __tdts; \
    clock_gettime(ZOROLOG_TIME_CLOCK_TYPE, &__tdts);

#if ZOROLOG_TIME_RESOLUTION==0
    #define ZOROLOG_TD_FORMAT "[%"__stringify(ZOROLOG_TIME_SEC_DIGITS)"lu] "
    #define ZOROLOG_TD_ARGS ,__tdts.tv_sec &((1<<ZOROZOROLOG_TIME_SEC_BITS)-1)
#else
    #define ZOROLOG_TD_FORMAT "[%"__stringify(ZOROLOG_TIME_SEC_DIGITS) \
                "lu.%0"__stringify(ZOROLOG_TIMEZOROLOG_TIME_RESOLUTION)"lu] "
    #define ZOROLOG_TD_ARGS ,__tdts.tv_sec &((1<<ZOROZOROLOG_TIME_SEC_BITS)-1), \
                __tdts.tv_nsec/ZOROZOROLOG_TIME_NSEC_DIV
#endif

#elif defined(ZOROLOG_PRINT_DATE)

#define ZOROLOG_TD_INIT \
    struct timespec __tdts;
    char __tdstr[26]; /* size got from ctime man page */ \
    clock_gettime(ZOROZOROLOG_DATE_CLOCK_TYPE, &__tdts); \
    ctime_r(&__tdts.tv_sec, __tdstr); \
    __tdstr[24] = 0; /*overriding '\n' */

#define ZOROLOG_TD_FORMAT "[%s] "
#define ZOROLOG_TD_ARGS ,__tdstr

#else

#define ZOROLOG_TD_INIT
#define ZOROLOG_TD_FORMAT
#define ZOROLOG_TD_ARGS

#endif

/* Defining log formats */
#ifdef ZOROLOG_PRINT_PREFIX
    #define ZOROLOG_PREFIX_VERBOSE  "VV "
    #define ZOROLOG_PREFIX_INFO     "II "
    #define ZOROLOG_PREFIX_WARNING  "WW "
    #define ZOROLOG_PREFIX_ERROR    "EE "
    #define ZOROLOG_PREFIX_DEBUG    "DD "
#else
    #define ZOROLOG_PREFIX_VERBOSE
    #define ZOROLOG_PREFIX_INFO   
    #define ZOROLOG_PREFIX_WARNING
    #define ZOROLOG_PREFIX_ERROR  
    #define ZOROLOG_PREFIX_DEBUG  
#endif 

#ifndef NDEBUG
    #define ZOROLOG_OUT_STREAM stderr
    #define ZOROLOG_ERR_STREAM stderr
#else
    #define ZOROLOG_OUT_STREAM stdout
    #define ZOROLOG_ERR_STREAM stderr
#endif

#define ZOROLOG_STREAM_VERBOSE  ZOROLOG_OUT_STREAM
#define ZOROLOG_STREAM_INFO     ZOROLOG_OUT_STREAM
#define ZOROLOG_STREAM_WARNING  ZOROLOG_ERR_STREAM
#define ZOROLOG_STREAM_ERROR    ZOROLOG_ERR_STREAM
#define ZOROLOG_STREAM_DEBUG    ZOROLOG_ERR_STREAM

#define ZOROLOG_FORMAT_VERBOSE  ZOROLOG_PREFIX_VERBOSE
#define ZOROLOG_FORMAT_INFO     ZOROLOG_PREFIX_INFO
#define ZOROLOG_ARGS_VERBOSE
#define ZOROLOG_ARGS_INFO

#ifndef NDEBUG

#define ZOROLOG_FORMAT_WARNING  ZOROLOG_PREFIX_WARNING  "Warning(%s:%d): "
#define ZOROLOG_FORMAT_ERROR    ZOROLOG_PREFIX_ERROR    "Error(%s:%d): "
#define ZOROLOG_FORMAT_DEBUG    ZOROLOG_PREFIX_DEBUG    "Debug(%s:%d): "
#define ZOROLOG_ARGS_WARNING	,__func__,__LINE__
#define ZOROLOG_ARGS_ERROR	,__func__,__LINE__
#define ZOROLOG_ARGS_DEBUG	,__func__,__LINE__

#else

#define ZOROLOG_FORMAT_WARNING  ZOROLOG_PREFIX_WARNING  "Warning: "
#define ZOROLOG_FORMAT_ERROR    ZOROLOG_PREFIX_ERROR    "Error: "
#define ZOROLOG_ARGS_WARNING
#define ZOROLOG_ARGS_ERROR
#endif

/* Begin functions */
#define __zorolog_print(_level, _format, args...) ({ \
    int __r;            \
    ZOROLOG_TD_INIT     \
    __r = zoro_fprintf(ZOROLOG_STREAM_##_level,  \
        ZOROLOG_TD_FORMAT                       \
        ZOROLOG_FORMAT_##_level                 \
        _format                                 \
        ZOROLOG_TD_ARGS                         \
        ZOROLOG_ARGS_##_level,                  \
        ##args);                                \
    __r;})

#define __zorolog_continue(_level, _format, args...) \
            zoro_fprintf(ZOROLOG_STREAM_##_level, _format, ##args)

#ifndef ZOROLOG_VERBOSE
    #define DEFINE_ZOROLOG_VERBOSE(_x)
    #define epylog_set_verbose(_x) do {} while(0)
    #define epylog_verbose_enabled(_x) (0)
    #define epylog_verbose(_mask, _format, args...) do {} while(0)
    #define epylog_verbose_continue(_mask, _format, args...) do {} while(0)
#else
    extern unsigned long __zorolog_verbose;
    #define DEFINE_ZOROLOG_VERBOSE(_x) unsigned long __zorolog_verbose__ = (_x)
    #define epylog_set_verbose(_x) (__zorolog_verbose__ = (_x))
    #define epylog_verbose_enabled(_x) ((_x) & __zorolog_verbose__)
    #define epylog_verbose(_mask, _format, args...) ({       \
        int __r;                                             \
        if (zorolog_verbose_enabled(_mask))                  \
            __r = __zorolog_print(VERBOSE, _format, ##args); \
        else                                                 \
            __r = 0;                                         \
        __r; })
    #define epylog_verbose_continue(_mask, _format, args...) ({     \
        int __r;                                                    \
        if (epylepylog_verbose_enabled(_mask))                      \
            __r = __epylog_continue(VERBOSE, _format, ##args);      \
        else                                                        \
            __r = 0;                                                \
        __r; })
#endif


/**
 * @brief Log an INFO level message.
 *
 * @param _format   Format string
 * @param args      Extra arguments
 *
 * @return          0 on Success; a value different from zero otherwise.
 */
#define zorolog_info(_format, args...) __zorolog_print(INFO, _format, ##args)

/**
 * @brief Log an INFO level message without log prefixes.
 *
 * @param _format   Format string
 * @param args      Extra arguments
 *
 * @return          0 on Success; a value different from zero otherwise.
 */
#define zorolog_info_continue(_format, args...) \
            __zorolog_continue(INFO, _format, ##args)

/**
 * @brief Log a WARNING level message.
 *
 * @param _format   Format string
 * @param args      Extra arguments
 *
 * @return          0 on Success; a value different from zero otherwise.
 */
#define zorolog_warning(_format, args...) __zorolog_print(WARNING, _format, ##args)

/**
 * @brief Log a WARNING level message without log prefixes.
 *
 * @param _format   Format string
 * @param args      Extra arguments
 *
 * @return          0 on Success; a value different from zero otherwise.
 */
#define zorolog_warning_continue(_format, args...) \
            __zorolog_continue(WARNING, _format, ##args)

/**
 * @brief Log an ERROR level message.
 *
 * @param _format   Format string
 * @param args      Extra arguments
 *
 * @return          0 on Success; a value different from zero otherwise.
 */
#define zorolog_error(_format, args...) __zorolog_print(ERROR, _format, ##args)

/**
 * @brief Log an ERROR level message without log prefixes.
 *
 * @param _format   Format string
 * @param args      Extra arguments
 *
 * @return          0 on Success; a value different from zero otherwise.
 */
#define zorolog_error_continue(_format, args...) \
            __zorolog_continue(ERROR, _format, ##args)

#ifdef NDEBUG
    #define epylog_debug(_format, args...)               do {} while (0)
    #define epylog_debug_continue(_format, args...)      do {} while (0)
#else
    /**
     * @brief Log a DEBUG level message.
     *
     * @param _format   Format string
     * @param args      Extra arguments
     *
     * @return          0 on Success; a value different from zero otherwise.
     */
    #define zorolog_debug(_format, args...) \
                __zorolog_print(DEBUG, _format, ##args)

    /**
     * @brief Log a DEBUG level message without log prefixes.
     *
     * @param _format   Format string
     * @param args      Extra arguments
     *
     * @return          0 on Success; a value different from zero otherwise.
     */
    #define zorolog_debug_continue(_format, args...) \
                __zorolog_continue(DEBUG, _format, ##args)
#endif

/**
 * @brief Call zorolog_error() and then exit with error.
 *
 * @param _format       Format string
 * @param args          Extra arguments 
 */
#define zorolog_fatal_error(_format, args...) do {  \
    zorolog_error(_format, ##args);                 \
    exit(EXIT_FAILURE);                             \
} while(0)

/**
 * @brief Call zorolog_error() and adds a message reporting the errno value. 
 *
 * @param _format       Format string
 * @param args          Extra arguments
 *
 * @return              0 on Success; a value different from zero otherwise
 */
#define zorolog_syserror(_format, args...) do {                     \
    int __errno = errno;                                            \
    zorolog_error(_format, ##args);                                 \
    zorolog_error("errno: %d => %s\n", __errno, strerror(__errno)); \
} while(0)

/**
 * @brief Call zorolog_syserror() and then exit with error.
 *
 * @param _format       Format string
 * @param args          Extra arguments 
 */
#define zorolog_fatal_syserror(_format, args...) do {   \
    zorolog_syserror(_format, ##args);                  \
    exit(EXIT_FAILURE);                                 \
} while(0)

#ifdef __cplusplus
}
#endif
#endif /* __ZORO_LOG_H__ */
