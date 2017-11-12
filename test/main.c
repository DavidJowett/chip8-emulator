#include <check.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "chip8_test.h"
#include "runtime_error_test.h"


int main(int argc, char **argv){
        srand(time(NULL));
        SRunner *sr;
        int number_failed;

        sr = srunner_create(chip8_suite());
        srunner_add_suite(sr, runtime_error_suite());
        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);

        srunner_free(sr);

        return (number_failed == 0) ? 1 : 0;

        return 0;
}
