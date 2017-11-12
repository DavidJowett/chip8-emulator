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
