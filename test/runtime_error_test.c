#include <check.h>

#include "runtime_error_test.h"

#include "../src/runtime_error.h"

START_TEST(test_runtime_error_init){
        struct runtime_error *re = runtime_error_init("");
        ck_assert_ptr_nonnull(re);
        ck_assert_str_eq(re->msg, "");
        runtime_error_destroy(&re);
        ck_assert_ptr_null(re);

        re = runtime_error_init("An error has occured!");
        ck_assert_ptr_nonnull(re);
        ck_assert_str_eq(re->msg, "An error has occured!");
        runtime_error_destroy(&re);
        ck_assert_ptr_null(re);
}
END_TEST


Suite *runtime_error_suite(void){
        Suite *s;
        TCase *tc;
        s = suite_create("runtime_error");

        tc = tcase_create("core");
        
        tcase_add_test(tc, test_runtime_error_init);
        suite_add_tcase(s, tc);

        return s;
}
