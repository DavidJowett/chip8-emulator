/* Copyright (C) 2017 David Jowett
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */
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
