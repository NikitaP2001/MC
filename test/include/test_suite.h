#include "test_tools.h"

#include <assert.h>
#include <string.h>
#include <time.h>

#include <mc/types.h>

#define LEN_OF(str) (sizeof(str) - 1)

#define TEST_AF "Assertion failed: "
#define TEST_EF "Expectation failed: "

#define TEST_ASSERT_INFO printf("Assert failed on %s:%d.\n", __FILE__, __LINE__)
#define TEST_EXPECT_INFO printf("Expect failed on %s:%d.\n", __FILE__, __LINE__)

#define ASSERT_TRUE(arg)                                                        \
if (!arg) {                                                                     \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg" (%llx) <=> true\n", (mc_u64_t)arg);                        \
        *__test_case_status__ = false;                                          \
        return;                                                                 \
}
#define ASSERT_FALSE(arg)                                                       \
if (arg) {                                                                      \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg" (%llx) <=> false\n", (mc_u64_t)arg);                       \
        *__test_case_status__ = false;                                          \
        return;                                                                 \
}
#define ASSERT_EQ(arg1, arg2)                                                   \
if (arg1 != arg2) {                                                             \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg1" (%llx) != "#arg2" (%llx)\n",                              \
                (mc_u64_t)arg1, (mc_u64_t)arg2);                                \
        *__test_case_status__ = false;                                          \
        return;                                                                 \
}

#define ASSERT_NE(arg1, arg2)                                                   \
if (arg1 == arg2) {                                                             \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg1" (%llx) == "#arg2" (%llx)\n",                              \
                (mc_u64_t)arg1, (mc_u64_t)arg2);                                \
        *__test_case_status__ = false;                                          \
        return;                                                                 \
}
#define ASSERT_STR_EQ(arg1, arg2)                                               \
if (strcmp(arg1, arg2) != 0) {                                                  \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg1" (%s) <=> "#arg2" (%s)\n", arg1, arg2);                    \
        *__test_case_status__ = false;                                          \
        return;                                                                 \
}

/* TODO: rewrite all macros to fputs like this one */
#define EXPECT_EQ(arg1, arg2)                                                   \
if (arg1 != arg2) {                                                             \
        TEST_EXPECT_INFO;                                                       \
        fputs(#arg1, stdout);                                                   \
        printf(" (%llx) != ", (mc_u64_t)arg1);                                  \
        fputs(#arg2, stdout);                                                   \
        printf(" (%llx)\n", (mc_u64_t)arg2);                                    \
        *__test_case_status__ = false;                                          \
}
#define EXPECT_NE(arg1, arg2)                                                   \
if (arg1 == arg2) {                                                             \
        TEST_EXPECT_INFO;                                                       \
        printf(#arg1" (%llx) == "#arg2" (%llx)\n",                              \
                (mc_u64_t)arg1, (mc_u64_t)arg2);                                \
        *__test_case_status__ = false;                                          \
}
#define EXPECT_STR_EQ(arg1, arg2)                                               \
if (strcmp(arg1, arg2) != 0) {                                                  \
        TEST_EXPECT_INFO;                                                       \
        printf(#arg1" (%s) <=> "#arg2" (%s)\n", arg1, arg2);                    \
        *__test_case_status__ = false;                                          \
}

#define EXPECT_TRUE(arg)                                                        \
if (arg != true) {                                                              \
        TEST_EXPECT_INFO;                                                       \
        printf(#arg" (%llx) <=> true\n", (mc_u64_t)arg);                        \
        *__test_case_status__ = false;                                          \
}

#define TEST_FAILTURE(...)                                                      \
        printf(__VA_ARGS__);                                                    \
        *__test_case_status__ = false;                                          \
        return;


static inline double test_get_time()
{
        double time = (double)clock();
	return time / CLOCKS_PER_SEC * 1000;
}

#define TCASE_NAME(module, case) module ## _ ## case
/* Renamed argument from result to __test_case_status__ */
#define TCASE_DEF(module, case) TCASE_NAME(module, case) (_Bool *__test_case_status__)
#define TEST_RUN(module, case) case_ ## module ## _ ## case ()

#ifndef _TEST_RESULT_
#define _TEST_RESULT_
static _Bool g_test_result = true;
#endif /* _TEST_RESULT_ */
#define TEST_RESULT (g_test_result ? 0 : 1)

typedef long unsigned int test_time_t;

#define TEST_CASE(module, case)                                                \
static void TCASE_DEF(module, case);                                           \
static void TEST_RUN(module, case)                                             \
{                                                                              \
        /* Renamed variable from result to __test_case_status__ */             \
        _Bool __test_case_status__ = true;                                     \
        double t_ms = test_get_time();                                         \
        /* Pass the renamed variable */                                        \
        TCASE_NAME(module, case)(&__test_case_status__);                       \
        t_ms = test_get_time() - t_ms;                                         \
        /* Check the renamed variable */                                       \
        const char *str_res = __test_case_status__ ? "true" : "false";         \
        printf(""#module":"#case":%s:%lu\n", str_res, (test_time_t)t_ms);      \
        fflush(stdout);                                                        \
        /* Update global result with the renamed variable */                   \
        g_test_result &= __test_case_status__;                                 \
}                                                                              \
/* Use the renamed argument in the function definition */                      \
static void TCASE_DEF(module, case)
