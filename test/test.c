#include <time.h>
#include <stdlib.h>

#include <check.h>

#include "check_reader.h"
#include "check_snippet.h"

struct config {
        size_t power;
};

struct config default_config()
{
        struct config result = {0};
        result.power = 1;
        return result;
}

void init_rand()
{
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        srand(ts.tv_nsec);
}

struct config parse_args(int argc, char *argv[])
{
        struct config result = default_config();
        for (int i = 0; i < argc; i++) {
                if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                        result.power = atoll(argv[i + 1]);
                        i += 1;
                }
        }
        return result;
}

int main(int argc, char *argv[])
{
        int number_failed = 0;
        Suite *s_reader = NULL;
        Suite *s_snippet = NULL;
        SRunner *sr = NULL;

        init_rand();

        s_reader = reader_suite(parse_args(argc, argv).power);
        s_snippet = snippet_suite();
        sr = srunner_create(s_reader);
        srunner_add_suite(sr, s_snippet);

        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
