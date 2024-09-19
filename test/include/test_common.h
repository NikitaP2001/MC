#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define TFILE_NAME "file.cc"

#define LEN_OF(str) (sizeof(str) - 1)

#define TEST_AF "Assertion failed: "
#define TEST_EF "Expectation failed: "

#define TEST_ASSERT_INFO printf("Assert failed on %s:%d.\n", __FILE__, __LINE__)
#define TEST_EXPECT_INFO printf("Expect failed on %s:%d.\n", __FILE__, __LINE__)

#define ASSERT_TRUE(arg)                                                        \
if (!arg) {                                                                     \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg" (%llx) <=> false\n", (mc_u64_t)arg);                       \
        *result = false;                                                        \
        return;                                                                 \
}
#define ASSERT_FALSE(arg)                                                       \
if (!arg) {                                                                     \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg" (%llx) <=> true\n", (mc_u64_t)arg);                        \
        *result = false;                                                        \
        return;                                                                 \
}
#define ASSERT_EQ(arg1, arg2)                                                   \
if (arg1 != arg2) {                                                             \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg1" (%llx) != "#arg2" (%llx)\n",                              \
                (mc_u64_t)arg1, (mc_u64_t)arg2);                                \
        *result = false;                                                        \
        return;                                                                 \
}

#define ASSERT_NE(arg1, arg2)                                                   \
if (arg1 == arg2) {                                                             \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg1" (%llx) == "#arg2" (%llx)\n",                              \
                (mc_u64_t)arg1, (mc_u64_t)arg2);                                \
        *result = false;                                                        \
        return;                                                                 \
}
#define ASSERT_STR_EQ(arg1, arg2)                                               \
if (strcmp(arg1, arg2) != 0) {                                                  \
        TEST_ASSERT_INFO;                                                       \
        printf(#arg1" (%s) <=> "#arg2" (%s)\n", arg1, arg2);                    \
        *result = false;                                                        \
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
        *result = false;                                                        \
}
#define EXPECT_NE(arg1, arg2)                                                   \
if (arg1 == arg2) {                                                             \
        TEST_EXPECT_INFO;                                                       \
        printf(#arg1" (%llx) == "#arg2" (%llx)\n",                              \
                (mc_u64_t)arg1, (mc_u64_t)arg2);                                \
        *result = false;                                                        \
}
#define EXPECT_STR_EQ(arg1, arg2)                                               \
if (strcmp(arg1, arg2) != 0) {                                                  \
        TEST_EXPECT_INFO;                                                       \
        printf(#arg1" (%s) <=> "#arg2" (%s)\n", arg1, arg2);                    \
        *result = false;                                                        \
}

static inline double test_get_time()
{
        double time = (double)clock();
	return time / CLOCKS_PER_SEC * 1000;
}

#define TCASE_NAME(module, case) module ## _ ## case
#define TCASE_DEF(module, case) TCASE_NAME(module, case) (_Bool *result)
#define TEST_RUN(module, case) case_ ## module ## _ ## case ()

#define TEST_CASE(module, case)                                                \
static void TCASE_DEF(module, case);                                           \
static void TEST_RUN(module, case)                                             \
{                                                                              \
        _Bool result = true;                                                   \
        printf("[ RUN      ] "#module"."#case"\n");                            \
        double t_ms = test_get_time();                                         \
        TCASE_NAME(module, case)(&result);                                     \
        t_ms = test_get_time() - t_ms;                                         \
        if (result)                                                            \
                printf("[       OK ] "#module"."#case" (%f ms)\n", t_ms);      \
        else                                                                   \
                printf("[  FAILED  ] "#module"."#case" (%f ms)\n", t_ms);      \
}                                                                              \
static void TCASE_DEF(module, case)

static inline 
_Bool 
write_file(const char *file_name, const char *content, size_t length)
{
        _Bool result = false;
        FILE *fp = fopen(file_name, "wb");
        if (fp != NULL) {
                fwrite(content, sizeof(char), length, fp);

                result = ferror(fp) == 0;
                fclose(fp);
        }
        return result; 
}