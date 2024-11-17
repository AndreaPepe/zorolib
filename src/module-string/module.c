/**
 * @author Andrea Pepe <pepe.andmj@gmail.com>
 * @copyright Copyright (c) 2024
 */

#define _GNU_SOURCE

#include <zoro/string.h>
#include <zoro/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

char * zorostr_vsprintf(const char *format, va_list ap)
{
    char *str;
    if (vasprintf(&str, format, ap) == -1)
        return NULL;
    return str;
}

int zorostr_strdup(char **dest, const char *newstring)
{
    char *tmp;
    size_t len;

    if (dest == NULL)
        return -1;

    if (newstring == NULL){
        if (*dest != NULL){
            free(*dest);
            *dest=NULL;
        }
        return 0;
    }

    len = strlen(newstring) + 1;
    tmp = (char *)malloc(len);
    if (tmp == NULL)
        return -1;

    memcpy(tmp, newstring, len);

    if (*dest)
        free(*dest);
    *dest = tmp;

    return 0;
}

char *zorostr_get_random_string(size_t len, const char *prefix)
{
    size_t plen;
    char *s;
    struct timespec now;

    if (len == 0)
        return NULL;

    if (prefix) {
        plen = strlen(prefix);
        len += plen + 1;
    } else {
        plen = 0;
    }

    s = (char *)malloc((len + 1) * sizeof(char));
    if (s == NULL)
        return NULL;

    if (prefix) {
        strcpy(s, prefix);
        s[plen++] = '-';
    }

    // Set seed for PRNG using current time and hostname
    clock_gettime(CLOCK_MONOTONIC, &now);
    now.tv_nsec += gethostid();
    srand((unsigned int)(now.tv_nsec));

    while(plen < len)
        s[plen++] = 'A' + (rand() % ('Z' - 'A' + 1));

    s[len] = 0;

    return s;
}


/*** Begin CONVERSIONS ****/

int __zorostr_strtoi(const char *s, int *res, char **endptr)
{
    long int tmp;

    if (s == NULL || res == NULL)
        return -1;

    if (__zorostr_strtol(s, &tmp, endptr) != 0)
        return -1;

    if (tmp > INT_MAX || tmp < INT_MIN)
        return -1;

    (*res) = (int)tmp;

    return 0;
}

int __zorostr_strtoui(const char *s, unsigned int *res, char **endptr)
{
    unsigned long int tmp;

    if (s == NULL || res == NULL)
        return -1;

    if (__zorostr_strtoul(s, &tmp, endptr) != 0)
        return -1;

    if (tmp > UINT_MAX)
        return -1;

    (*res) = (unsigned int)tmp;

    return 0;
}

int __zorostr_strtos(const char *s, short *res, char **endptr)
{
    long int tmp;

    if (s == NULL || res == NULL)
        return -1;

    if (__zorostr_strtol(s, &tmp, endptr) != 0)
        return -1;

    if (tmp > SHRT_MAX || tmp < SHRT_MIN)
        return -1;

    (*res) = (short)tmp;

    return 0;
}

int __zorostr_strtous(const char *s, unsigned short *res, char **endptr)
{
    unsigned long int tmp;

    if (s == NULL || res == NULL)
        return -1;

    if (__zorostr_strtoul(s, &tmp, endptr) != 0)
        return -1;

    if (tmp > USHRT_MAX)
        return -1;

    (*res) = (unsigned short)tmp;

    return 0;
}

int __zorostr_strtol(const char *s, long *res, char **endptr)
{
    char *end;
    long int tmp;

    if (s==NULL || res == NULL || s[0] == '\0')
        return -1;

    errno = 0;

    tmp = strtol(s, &end, 0);
    if ((errno == ERANGE && (tmp == LONG_MAX || tmp == LONG_MIN)) ||
        (errno != 0 && tmp == 0))
        return -1;

    if (endptr == NULL){
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    (*res) = tmp;

    return 0;
}

int __zorostr_strtoul(const char *s, unsigned long *res, char **endptr)
{
    char *end;
    unsigned long int tmp;

    if (s==NULL || res == NULL || s[0] == '\0' || s[0] == '-')
        return -1;

    errno = 0;

    tmp = strtoul(s, &end, 0);
    if ((errno == ERANGE && tmp == ULONG_MAX) || (errno != 0 && tmp == 0))
        return -1;

    if (endptr == NULL){
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    (*res) = tmp;

    return 0;
}

int __zorostr_strtoll(const char *s, long long *res, char **endptr)
{
    char *end;
    long long int tmp;

    if (s==NULL || res == NULL || s[0] == '\0')
        return -1;

    errno = 0;

    tmp = strtoll(s, &end, 0);
    if ((errno == ERANGE && (tmp == LLONG_MAX || tmp == LLONG_MIN)) ||
        (errno != 0 && tmp == 0))
        return -1;

    if (endptr == NULL){
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    (*res) = tmp;

    return 0;
}

int __zorostr_strtoull(const char *s, unsigned long long *res, char **endptr)
{
    char *end;
    unsigned long long int tmp;

    if (s==NULL || res == NULL || s[0] == '\0' || s[0] == '-')
        return -1;

    errno = 0;

    tmp = strtoull(s, &end, 0);
    if ((errno == ERANGE && tmp == ULLONG_MAX) || (errno != 0 && tmp == 0))
        return -1;

    if (endptr == NULL){
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    (*res) = tmp;

    return 0;
}

int __zorostr_strtof(const char *s, float *res, char **endptr)
{
    char *end;
    float tmp;

    if (s == NULL || res == NULL || s[0] == '\0')
        return -1;

    errno = 0;

    tmp = strtof(s, &end);
    if ((errno == ERANGE && (tmp == -HUGE_VALF || tmp == HUGE_VALF)) ||
        (errno != 0 && tmp == 0.0))
        return -1;

    if (endptr == NULL) {
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    *res = tmp;

    return 0;
}

int __zorostr_strtod(const char *s, double *res, char **endptr)
{
    char *end;
    double tmp;

    if (s == NULL || res == NULL || s[0] == '\0')
        return -1;

    errno = 0;

    tmp = strtod(s, &end);
    if ((errno == ERANGE && (tmp == -HUGE_VAL || tmp == HUGE_VAL)) ||
        (errno != 0 && tmp == 0.0))
        return -1;

    if (endptr == NULL) {
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    *res = tmp;

    return 0;
}

int __zorostr_strtold(const char *s, long double *res, char **endptr)
{
    char *end;
    long double tmp;

    if (s == NULL || res == NULL || s[0] == '\0')
        return -1;

    errno = 0;

    tmp = strtold(s, &end);
    if ((errno == ERANGE && (tmp == -HUGE_VALL || tmp == HUGE_VALL)) ||
        (errno != 0 && tmp == 0.0))
        return -1;

    if (endptr == NULL) {
        if (end[0] != '\0')
            return -1;
    } else
        (*endptr) = end;

    *res = tmp;

    return 0;
}

int __zorostr_strtoc(const char *s, char *res, char **endptr)
{
    long int tmp;

    if (s == NULL || res == NULL)
        return -1;

    if (__zorostr_strtol(s, &tmp, endptr) != 0)
        return -1;

    if (tmp > CHAR_MAX || tmp < CHAR_MIN)
        return -1;

    (*res) = (char)tmp;

    return 0;
}

int __zorostr_strtouc(const char *s, unsigned char *res, char **endptr)
{
    unsigned long int tmp;

    if (s == NULL || res == NULL)
        return -1;

    if (__zorostr_strtoul(s, &tmp, endptr) != 0)
        return -1;

    if (tmp > UCHAR_MAX)
        return -1;

    (*res) = (unsigned char)tmp;

    return 0;
}
