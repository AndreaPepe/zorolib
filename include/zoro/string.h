/**
 * @file string.h
 * @copyright Copyright (c) 2024
 * @author Andrea Pepe <pepe.andmj@gmail.com>
 *
 * @brief String utilities.
 *
 * Advanced string API wrappers.
 */

#pragma once
#ifndef __ZORO_STRING_H__
#define __ZORO_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * @brief Return the formatted output as a allocated string.
 *
 * This function is like vsprintf, but it returns a newly allocated string.
 * You should consider using vasprintf.
 *
 * @param format The output format string.
 * @param ap     List of additional arguments.
 * @return On success, return the newly allocated string; on error, return
 *         NULL.
 */
char *zorostr_vsprintf(const char *format, va_list ap);

/**
 * @brief Return the formatted output as a allocated string.
 *
 * This function is like sprintf, but it returns a newly allocated string.
 * You should consider using asprintf.
 *
 * @param format The output format string.
 * @return On success, return the newly allocated string; on error, return
 *         NULL.
 */
static inline char *zorostr_sprintf(const char *format, ...)
{
    va_list ap;
    char *ret;
    va_start(ap, format);
    ret = zorostr_vsprintf(format, ap);
    va_end(ap);
    return ret;
}


/**
 * @brief Duplicate a string.
 *
 * This function is like @a strdup, but it assumes @a *dest is NULL or
 * allocated with @a malloc. If @a newstring is NULL, @a dest is (eventually)
 * freed.
 *
 * @param dest The destination string.
 * @param newstring The source string to copy.
 * @return Return 0 on success; -1 on error.
 */
int zorostr_strdup(char **dest, const char *newstring);

/**
 * @brief Return a random string.
 *
 * This function returns a newly allocated random string in capital letters.
 * If @a prefix is not NULL, the string will start with @a prefix, followed by
 * a '-' character and then @a len random capital letters.
 * The size of the output string is @a len + the length of @a prefix + 1.
 *
 * @param len The random string length excluding @a prefix.
 * @param prefix Prefix to the random string.
 * @return Return a random string on success; NULL in case of error. 
 */
char *zorostr_get_random_string(size_t len, const char *prefix);

/******************************* CONVERSIONS *********************************/

/**
 * @brief Converts an ASCII string to an integer (int).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtol
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtoi(const char *s, int *res, char **endptr);
#define zorostr_strtoi(_s, _res) __zorostr_strtoi(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to an unsigned integer (unsigned int).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtoui(const char *s, unsigned int *res, char **endptr);
#define zorostr_strtoui(_s, _res) __zorostr_strtoui(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a short integer (short).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtos(const char *s, short *res, char **endptr);
#define zorostr_strtos(_s, _res) __zorostr_strtos(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a unsigned short integer (unsigned 
 *        short).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtous(const char *s, unsigned short *res, char **endptr);
#define zorostr_strtous(_s, _res) __zorostr_strtous(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a long integer (long int).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtol
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtol(const char *s, long int *res, char **endptr);
#define zorostr_strtol(_s, _res) __zorostr_strtol(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a unsigned long integer (unsigned long).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtoul(const char *s, unsigned long *res, char **endptr);
#define zorostr_strtoul(_s, _res) __zorostr_strtoul(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a long long int.
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtoll(const char *s, long long int *res, char **endptr);
#define zorostr_strtoll(_s, _res) __zorostr_strtoll(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a unsigned long long int.
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtoull(const char *s, unsigned long long *res, char **endptr);
#define zorostr_strtoull(_s, _res) __zorostr_strtoull(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a floating point number (float).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtof
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtof(const char *s, float *res, char **endptr);
#define zorostr_strtof(_s, _res) __zorostr_strtof(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a double.
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtod
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtod(const char *s, double *res, char **endptr);
#define zorostr_strtod(_s, _res) __zorostr_strtod(_s, _res, NULL)

/**
 * @brief Converts an ASCII string to a long double.
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtold
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtold(const char *s, long double *res, char **endptr);
#define zorostr_strtold(_s, _res) __zorostr_strtold(_s, _res, NULL)


/**
 * @brief Converts an ASCII string to a character (char).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtol
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtoc(const char *s, char *res, char **endptr);
#define zorostr_strtoc(_s, _res) __zorostr_strtoc(_s, _res, NULL);

/**
 * @brief Converts an ASCII string to a unsigned character (unsigned char).
 *
 * Conversion fails if the input string is empty, not a valid number or the
 * numeric bound is not respected.
 * 
 * @see strtoul
 *
 * @param s The ASCII string.
 * @param res Returned value.
 * @param endptr If NULL, @a s must match exactly one number; otherwise, @a s
 *               may contain more characters and @a *endptr points to the first
 *               unused character.
 * @return Return 0 on success; -1 in case of error.
 */
int __zorostr_strtouc(const char *s, unsigned char *res, char **endptr);
#define zorostr_strtouc(_s, _res) __zorostr_strtouc(_s, _res, NULL);



#ifdef __cplusplus
}
#endif
#endif /* __ZORO_STRING_H__ */
