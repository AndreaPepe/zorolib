/**
 * @file test.h
 * @author Andrea Pepe
 * @copyright Copyright (c) 2024
 *
 * @brief Unit test and test-suite helpers. 
 */

#ifndef __ZORO_TEST_H__
#define __ZORO_TEST_H__

#include <zoro/log.h>
#include <zoro/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZOROTEST_SUCCESS 0
#define ZOROTEST_FAILURE -1

typedef int (*zorotest_test_t)(void);
typedef void (*zorotest_cleaner_t)(void *);

void *zorotest_to_clear = NULL;
zorotest_cleaner_t zorotest_fail_clean = NULL;
uint8_t zorotest_is_verbose = 0;

#define __zorotest_fail_msg() \
        zorolog_error("TEST '%s' FAILED!\n", PRETTY_FUNCTION)
/**
 * @brief Print a message if @a zorotest_is_verbose is set.
 *
 * @param _format       the message string; it can be a format string
 * @param args          arguments to fill the @a _format string
 */
#define zorotest_verbose(_format, args...)                              \
        do {                                                            \
                if (zorotest_is_verbose)                                \
                        zorolog_info(_format, ##args);                  \
        } while (0)                                                     \

/**
 * @brief Enable the zorotest verbose mode.
 */
#define zorotest_set_verbose() \
        zorotest_is_verbose = 1

/**
 * @brief Disable the zorotest verbose mode.
 */
#define zorotest_unset_verbose() \
        zorotest_is_verbose = 0

/**
 * @brief Register a cleaner function to be run in case of test failure.
 *
 * This function registers a function to clear a specific pointer.
 * The cleaner function pointer type must be @a zorotest_cleaner_t.
 * The @a __to_clear pointer type must be void *.
 * This should be used in a function of type @a zorotest_test_t.
 * This function can be called multiple times.
 * 
 * @warning Be aware that the object must be valid at the test failure time.
 *
 * @param __cleaner     the cleaner function; it must be of type @a
 *                      zorotest_cleaner_t
 * @param __to_clean    pointer to the object to clean
 */
#define zorotest_set_clear_on_fail(__cleaner, __to_clean)               \
        do {                                                            \
                zorotest_to_clear = __to_clean;                         \
                zorotest_fail_clean = __cleaner;                        \
        } while(0)

/**
 * @brief Unregister the registered cleaner function.
 * 
 * This function unregisters the cleaner function set with @a 
 * zorotest_set_clear_on_fail() and the relative object to clear.
 */
#define zorotest_unset_clear_on_fail()                                  \
        do {                                                            \
                zorotest_to_clear = NULL;                               \
                zorotest_fail_clean = NULL;                             \
        } while (0)

#define __zorotest_clear()                                              \
        do {                                                            \
                if (zorotest_fail_clean)                                \
                        zorotest_fail_clean(zorotest_to_clear);         \
        } while(0)


/**
 * @brief Fail the test.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * It makes the test to fail and prints the message @a __msg on the error log.
 * It also triggers the registered cleaner function set with @a 
 * zorotest_set_clear_on_fail(), if any.
 *
 * @param __msg         the message to print on error log     
 */
#define zorotest_fail(__msg)                                            \
        do {                                                            \
                __zorotest_fail_msg();                                  \
                zorolog_error(__msg);                                   \
                __zorotest_clear();                                     \
                return ZOROTEST_FAILURE;                                \
        } while(0)
       
/**
 * @brief Succeed the test.
 *
 * This function must be used inside a @a zorotest_test_t function. Every test
 * function should have a @a zorotest_success call somewhere.
 * It also unregisters any previously set cleaner function. 
 */
#define zorotest_success()                                              \
        do {                                                            \
                zorotest_unset_clear_on_fail();                         \
                return ZOROTEST_SUCCESS;                                \
        } while(0)

/**
 * @brief Assert two numbers are equal in a test.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If @a __act is not equalt to @a __exp, it causes the test to fail and print
 * an error message on the error log.
 * If zorotest verbose mode is enabled, it also prints a message on log.
 *
 * @param __exp         the expected value
 * @param __act         the actual value
 * @param __fmt         the number format string (e.g. "%d", "%lu", ...)
 */
#define zorotest_assert_eq_nums(__exp, __act, __fmt)                    \
        do {                                                            \
                if ((__exp) != (__act)) {                               \
                        __zorotest_fail_msg();                          \
                        zorolog_error("Expected " __fmt " Actual " __fmt "\n",\
                                      __exp, __act);                    \
                        __zorotest_clear();                             \
                        return ZOROTEST_SUCCESS;                        \
                }                                                       \
                zorotest_verbose("Number = " __fmt " - Exact\n", __exp);\
        } while(0)

/**
 * @brief Assert two strings are equal in a test.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If @a __act is not equalt to @a __exp, it causes the test to fail and print
 * an error message on the error log.
 * If zorotest verbose mode is enabled, it also prints a message on log.
 *
 * @note If both strings are NULL, the assert is evaluated to true.
 *
 * @param __exp         the expected value
 * @param __act         the actual value
 */
#define zorotest_assert_eq_strings(__exp, __act)                        \
        do {                                                            \
                if(!((__exp) == NULL && (__act) == NULL)) {             \
                        if ((__exp) == NULL || (__act) == NULL ||       \
                            strcmp(__exp, __act) != 0) {                \
                                __zorotest_fail_msg();                  \
                                zorolog_error("Expected %s\n Actual %s\n",\
                                              __exp, __act);            \
                                __zorotest_clear();                     \
                                return ZOROTEST_SUCCESS;                \
                        }                                               \
                }                                                       \
                zorotest_verbose("String = %s - Exact\n", __exp);       \
        } while(0)

/**
 * @brief Assert two memory areas are equal in a test.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If @a __act is not equalt to @a __exp, it causes the test to fail and print
 * an error message on the error log.
 * If zorotest verbose mode is enabled, it also prints a message on log.
 *
 * @param __exp         the expected value
 * @param __act         the actual value
 * @param __len         the length in bytes of the memory areas
 */
#define zorotest_assert_eq_mem(__exp, __act, __len)                     \
        do {                                                            \
                if (memcmp(__exp, __act, __len) != 0) {                 \
                        __zorotest_fail_msg();                          \
                        zorolog_error("Expected %s\n Actual %s\n",      \
                                      __exp, __act);                    \
                        __zorotest_clear();                             \
                        return ZOROTEST_SUCCESS;                        \
                }                                                       \
                zorotest_verbose("Memory (as string) = %s - Exact\n",   \
                                 __exp);                                \
        } while(0)

/**
 * @brief Assert two dynamic arrays of numbers are equals in a test.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If @a __act is not equalt to @a __exp, it causes the test to fail and print
 * an error message on the error log.
 * If zorotest verbose mode is enabled, it also prints a message on log.
 *
 * @param __exp         the expected result
 * @param __act         the actual result
 * @param __fmt         the print format for the numbers
 * @param __num         the numbers of elements to test
 */
#define zorotest_assert_eq_vector(__exp, __act, __fmt, __num)		\
        do {								\
                size_t __i = 0;						\
                                                                        \
                for (__i = 0; __i < __num; __i++) {			\
                        if ((__exp)[__i] != (__act)[__i]) {		\
                                __zorotest_fail_msg();			\
                                zorolog_error("Pos %lu Expected " __fmt	\
                                              "Actual " __fmt "\n",	\
                                              __i, (__exp)[__i],        \
                                              (__act)[__i]);	        \
                                __zorotest_clear();			\
                                return ZOROTEST_FAILURE;		\
                        }						\
                        zorotest_verbose("v[%lu] = " __fmt " Exact\n",  \
                                        __i, (__exp)[__i]);		\
                }							\
        } while (0)

/**
 * @brief Assert two static arrays of numbers are equals in a test.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If @a __act is not equalt to @a __exp, it causes the test to fail and print
 * an error message on the error log.
 * If zorotest verbose mode is enabled, it also prints a message on log.
 *
 * @param __exp         the expected result
 * @param __act         the actual result
 * @param __fmt         the print format for the numbers
 */
#define zorotest_assert_eq_array(__exp, __act, __fmt)   		\
        do {								\
                zorotest_assert_eq_nums(ARRAY_SIZE(__exp),              \
                                        ARRAY_SIZE(__act),              \
                                        "array size %lu");              \
                zorotest_assert_eq_vector(__exp, __act, __fmt,          \
                                          ARRAY_SIZE(__exp));           \
        } while (0)

#define __zorotest_assert_cond(__cond, __fmt, __str)                    \
        do {                                                            \
                if (!(__cond)) {                                        \
                        __zorotest_fail_msg();			        \
                        zorolog_error(__fmt, __str);                    \
                        __zorotest_clear();			        \
                        return ZOROTEST_FAILURE;		        \
                }                                                       \
        } while (0)

/**
 * @brief Assert a condition is true.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If the condition @a __cond is not true, it causes the test failure and 
 * prints an error message on the error log.
 *
 * @param __cond        the condition to evaluate
 */
#define zorotest_assert_true(__cond)					\
        do {								\
                __zorotest_assert_cond(__cond,                          \
                                       "Expected '%s' as true\n",	\
                                      __stringify_1(__cond));		\
        } while (0)

/**
 * @brief Assert a condition is false.
 *
 * This function must be used inside a @a zorotest_test_t function.
 * If the condition @a __cond is not false, it causes the test failure and 
 * prints an error message on the error log.
 *
 * @param __cond        the condition to evaluate
 */
#define zorotest_assert_false(__cond)					\
        do {								\
                __zorotest_assert_cond(!(__cond),                       \
                                       "Expected '%s' as false\n",	\
                                       __stringify_1(__cond));		\
        } while (0)

static inline int __zorotest_run_test_suite(zorotest_test_t tests[],
                                            size_t num_tests,
                                            const char *test_suite_name)
{
        size_t i;

        zorolog_info("RUNNING test suite %s\n", test_suite_name);
        for (i = 0; i < num_tests; i++) {
                zorotest_verbose("Running test n# %lu ...", i);
                if (tests[i]() != ZOROTEST_SUCCESS) {
                        zorolog_error("\nTEST SUITE '%s' FAILED!\n", 
                                      test_suite_name);
                        return ZOROTEST_FAILURE;
                }
                zorotest_verbose("PASS!\n");
        }
        zorolog_info("TEST SUITE %s ...PASS!\n", test_suite_name);
        return ZOROTEST_SUCCESS;
}

#define zorotest_run_test_suite(__tests, __test_suite_name)             \
        __zorotest_run_test_suite(__tests, ARRAY_SIZE(__tests),         \
                                  __test_suite_name)

#ifdef __cplusplus
}
#endif
#endif /* __ZORO_TEST_H__ */
