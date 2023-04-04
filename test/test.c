#include <time.h>
#include <stdlib.h>

#include <check.h>

#include "check_reader.h"

void init_rand()
{
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        srand(ts.tv_nsec);
}


int main(void)
{
        int number_failed = 0;
        Suite *s = NULL;
        SRunner *sr = NULL;

        init_rand();

        s = reader_suite();
        sr = srunner_create(s);

        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);

        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
