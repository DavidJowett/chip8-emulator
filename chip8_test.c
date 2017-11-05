#include <check.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "chip8.h"


START_TEST(test_mState_create){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->stackSize, 0);
        ck_assert_int_eq(ms->pc, 0);
        ck_assert_int_eq(ms->iRegister, 0);
        for(int i = 0; i < 16; i++)
                ck_assert_int_eq(ms->registers[1], 0);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_mState_delete){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        delete_mState(&ms);
        ck_assert_ptr_null(ms);
}
END_TEST

/* Test clear display instruction:
 * 0x0E0 clears the display */
START_TEST(test_clear_insturction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        run_instruction(ms, 0x00E0);
        for(int x = 0; x < 64; x++)
                for(int y = 0; y < 32; y++)
                        ck_assert_uint_eq(ms->disp[x][y], 0x0);
        for(int x = 0; x < 64; x++)
                for(int y = 0; y < 32; y++)
                        ms->disp[x][y] = 1;
        run_instruction(ms, 0x00E0);
        for(int x = 0; x < 64; x++)
                for(int y = 0; y < 32; y++)
                        ck_assert_uint_eq(ms->disp[x][y], 0x0);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_return_instruction){
        struct mState *ms;
        ms = create_mState();
        ms->stack[0] = 0x1212;
        ms->stackSize = 1;
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x00EE);
        ck_assert_int_eq(ms->pc, 0x1212);
        ck_assert_int_eq(ms->stackSize, 0);
        delete_mState(&ms);
}
END_TEST

/* Test Jump instruction:
 * 0x1NNN jump to 12 bit immediate value */
START_TEST(test_jump_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x1456);
        ck_assert_int_eq(ms->pc, 0x456);
        run_instruction(ms, 0x1956);
        ck_assert_int_eq(ms->pc, 0x956);
        run_instruction(ms, 0x1156);
        ck_assert_int_eq(ms->pc, 0x156);
}
END_TEST

START_TEST(test_subroutine_call_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x2123);
        ck_assert_int_eq(ms->pc, 0x123);
        ck_assert_int_eq(ms->stackSize, 1);
        ck_assert_int_eq(ms->stack[0], 0x2);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_equal_immediate){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x3000);
        ck_assert_int_eq(ms->pc, 4);
        run_instruction(ms, 0x3034);
        ck_assert_int_eq(ms->pc, 6);
        ms->registers[7] = 0x82;
        run_instruction(ms, 0x3782);
        ck_assert_uint_eq(ms->pc, 10);
        run_instruction(ms, 0x3772);
        ck_assert_uint_eq(ms->pc, 12);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_not_equal_immediate){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x4000);
        ck_assert_int_eq(ms->pc, 2);
        run_instruction(ms, 0x4001);
        ck_assert_int_eq(ms->pc, 6);
        ms->registers[8] = 0x89;
        run_instruction(ms, 0x4889);
        ck_assert_int_eq(ms->pc, 8);
        run_instruction(ms, 0x4890);
        ck_assert_int_eq(ms->pc, 12);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_equal){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x5010);
        ck_assert_int_eq(ms->pc, 4);
        ms->registers[0xA] = 1;
        run_instruction(ms, 0x50A0);
        ck_assert_int_eq(ms->pc, 6);
        ms->registers[0xB] = 0x89;
        ms->registers[0x7] = 0x89;
        run_instruction(ms, 0x5AB0);
        ck_assert_int_eq(ms->pc, 8);
        run_instruction(ms, 0x5B70);
        ck_assert_int_eq(ms->pc, 12);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_load_immediate_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        run_instruction(ms, 0x6AFF);
        ck_assert_uint_eq(ms->registers[0xA], 0xFF);
        run_instruction(ms, 0x6A11);
        ck_assert_uint_eq(ms->registers[0xA], 0x11);
        run_instruction(ms, 0x6111);
        ck_assert_uint_eq(ms->registers[0x1], 0x11);
        run_instruction(ms, 0x6099);
        ck_assert_uint_eq(ms->registers[0x0], 0x99);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_add_immediate_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        run_instruction(ms, 0x7011);
        ck_assert_uint_eq(ms->registers[0x0], 0x11);
        run_instruction(ms, 0x7011);
        ck_assert_uint_eq(ms->registers[0x0], 0x22);
        run_instruction(ms, 0x7A13);
        ck_assert_uint_eq(ms->registers[0xA], 0x13);
        run_instruction(ms, 0x7A0F);
        ck_assert_uint_eq(ms->registers[0xA], 0x22);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_move_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ms->registers[0x0] = 0x99;
        run_instruction(ms, 0x8A00);
        ck_assert_uint_eq(ms->registers[0xA], 0x99);
        run_instruction(ms, 0x8010);
        ck_assert_uint_eq(ms->registers[0x0], 0x00);
        run_instruction(ms, 0x8DA0);
        ck_assert_uint_eq(ms->registers[0xD], 0x99);
        run_instruction(ms, 0x8A00);
        ck_assert_uint_eq(ms->registers[0xA], 0x00);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_or_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ms->registers[0x0] = 0x99;
        run_instruction(ms, 0x8101);
        ck_assert_uint_eq(ms->registers[0x1], 0x99);
        ck_assert_uint_eq(ms->registers[0x0], 0x99);
        ms->registers[0x2] = 0x01;
        ms->registers[0x3] = 0x02;
        run_instruction(ms, 0x8321);
        ck_assert_uint_eq(ms->registers[0x3], 0x03);
        ck_assert_uint_eq(ms->registers[0x2], 0x01);
        run_instruction(ms, 0x8AB1);
        ck_assert_uint_eq(ms->registers[0xB], 0x0);
        ms->registers[0x9] = 0xF7;
        run_instruction(ms, 0x8991);
        ck_assert_uint_eq(ms->registers[0x9], 0xF7);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_and_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ms->registers[0x0] = 0xF8;
        run_instruction(ms, 0x8102);
        ck_assert_uint_eq(ms->registers[0x1], 0x0);
        ms->registers[0xA] = 0x8F;
        run_instruction(ms, 0x8A02);
        ck_assert_uint_eq(ms->registers[0xA], 0x88);
        ms->registers[0x5] = 0xFF;
        ms->registers[0x4] = 0xFF;
        run_instruction(ms, 0x8542);
        ck_assert_uint_eq(ms->registers[0x5], 0xFF);
        ms->registers[0x4] = 0x11;
        run_instruction(ms, 0x8542);
        ck_assert_uint_eq(ms->registers[0x5], 0x11);
        delete_mState(&ms);
}
END_TEST

START_TEST(test_xor_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ms->registers[0x0] = 0x11;
        run_instruction(ms, 0x8103);
        ck_assert_uint_eq(ms->registers[0x1], 0x11);
        ms->registers[0x2] = 0x11;
        ms->registers[0x3] = 0x11;
        run_instruction(ms, 0x8323);
        ck_assert_uint_eq(ms->registers[0x3], 0x00);

        delete_mState(&ms);
}
END_TEST

START_TEST(test_add_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        /* check that addition works and that Rf is set to 0 when no integer
         * role over occurs */
        ms->registers[0x0] = 0x23;
        ms->registers[0x1] = 0x23;
        run_instruction(ms, 0x8104);
        ck_assert_uint_eq(ms->registers[0x0], 0x23);
        ck_assert_uint_eq(ms->registers[0x1], 0x46);
        ck_assert_uint_eq(ms->registers[0xF], 0x00);
        /* check that addition sets Rf to 1 when integer role over occurs */
        ms->registers[0xA] = 0xFF;
        ms->registers[0xB] = 0xFF;
        run_instruction(ms, 0x8AB4);
        ck_assert_uint_eq(ms->registers[0xB], 0xFF);
        ck_assert_uint_eq(ms->registers[0xA], 0xFE);
        ck_assert_uint_eq(ms->registers[0xF], 0x01);

        delete_mState(&ms);
}
END_TEST

/* Test the 0x8XY5 instruction */
START_TEST(test_subtract_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        /* check that subtraction works and that Rf is set to 1 when no carry
         * occurs */
        ms->registers[0x0] = 0x65;
        ms->registers[0x1] = 0x44;
        run_instruction(ms, 0x8015);
        ck_assert_uint_eq(ms->registers[0x0], 0x21);
        ck_assert_uint_eq(ms->registers[0x1], 0x44);
        ck_assert_uint_eq(ms->registers[0xF], 0x01);

        /* check that Rf is set to 0 when carry occurs */
        ms->registers[0x5] = 0x10;
        ms->registers[0x4] = 0x60;
        run_instruction(ms, 0x8545);
        ck_assert_uint_eq(ms->registers[0x4], 0x60);
        ck_assert_uint_eq(ms->registers[0x5], 0xB0);
        ck_assert_uint_eq(ms->registers[0xF], 0x00);

        delete_mState(&ms);
}
END_TEST

/* Test the right shift instruction
 * 0x8XY6 shifts Vy to the right by 1, assign the value to Vx and sets Vf to the least
 * significant bit of Vy from before the shift */ 
START_TEST(test_right_shift_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);

        /* check the Vy is shifted to the right, that Vx is set to Vy after the
         * shift, and that Vf is set to the least significant bit of Vy from
         * before the shift. */
        ms->registers[0x0] = 0x01;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x8406);
        ck_assert_uint_eq(ms->registers[0x0], 0x0);
        ck_assert_uint_eq(ms->registers[0x1], 0x0);
        ck_assert_uint_eq(ms->registers[0xF], 0x1);

        /* Check that Vf is correctly set to 0 */
        ms->registers[0x0] = 0x02;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x8406);
        ck_assert_uint_eq(ms->registers[0x0], 0x1);
        ck_assert_uint_eq(ms->registers[0x4], 0x1);
        ck_assert_uint_eq(ms->registers[0xF], 0x0);

        /* One more test case */
        ms->registers[0x0] = 0xFF;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x8406);
        ck_assert_uint_eq(ms->registers[0x0], 0x7F);
        ck_assert_uint_eq(ms->registers[0x4], 0x7F);
        ck_assert_uint_eq(ms->registers[0xF], 0x1);

        delete_mState(&ms);
}
END_TEST

/* Test the Vx = Vy - Vx instruction
 * 0x8XY7 sets Vx to Vy - Vx and sets Vf to 1 if there is a carry or to 0 if
 * there is not carry */
START_TEST(test_subtract_yx_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        /* check that subtraction works and that Rf is set to 1 when no carry
         * occurs */
        ms->registers[0x1] = 0x65;
        ms->registers[0x0] = 0x44;
        run_instruction(ms, 0x8017);
        ck_assert_uint_eq(ms->registers[0x0], 0x21);
        ck_assert_uint_eq(ms->registers[0x1], 0x65);
        ck_assert_uint_eq(ms->registers[0xF], 0x01);

        /* check that Rf is set to 0 when carry occurs */
        ms->registers[0x4] = 0x10;
        ms->registers[0x5] = 0x60;
        run_instruction(ms, 0x8547);
        ck_assert_uint_eq(ms->registers[0x4], 0x10);
        ck_assert_uint_eq(ms->registers[0x5], 0xB0);
        ck_assert_uint_eq(ms->registers[0xF], 0x00);

        delete_mState(&ms);

}
END_TEST

/* Test the left shift instruction
 * 0x8XYE shifts Vy  to the left by 1, assign the value to Vx and sets Vf to the
 * most significant bit of Vy from before the shift */ 
START_TEST(test_left_shift_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);

        /* check the Vy is shifted to the left, that Vx is set to Vy after the
         * shift, and that Vf is set to the least significant bit of Vy from
         * before the shift. */
        ms->registers[0x0] = 0x01;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x840E);
        ck_assert_uint_eq(ms->registers[0x0], 0x2);
        ck_assert_uint_eq(ms->registers[0x4], 0x2);
        ck_assert_uint_eq(ms->registers[0xF], 0x0);

        /* Check that Vf is correctly set to 0 */
        ms->registers[0x0] = 0x02;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x840E);
        ck_assert_uint_eq(ms->registers[0x0], 0x4);
        ck_assert_uint_eq(ms->registers[0x4], 0x4);
        ck_assert_uint_eq(ms->registers[0xF], 0x0);

        /* Check that Vf is correctly set to 1 */
        ms->registers[0x0] = 0xFF;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x840E);
        ck_assert_uint_eq(ms->registers[0x0], 0xFE);
        ck_assert_uint_eq(ms->registers[0x4], 0xFE);
        ck_assert_uint_eq(ms->registers[0xF], 0x1);

        /* Check that Vf is correctly set to 1 */
        ms->registers[0x0] = 0x80;
        ms->registers[0x4] = 0x99;
        run_instruction(ms, 0x840E);
        ck_assert_uint_eq(ms->registers[0x0], 0x00);
        ck_assert_uint_eq(ms->registers[0x4], 0x00);
        ck_assert_uint_eq(ms->registers[0xF], 0x1);

        delete_mState(&ms);
}
END_TEST

/* Test the not equal operator
 * 0x9XY0 skips the next instruct if Vx != Vy */
START_TEST(test_not_equal_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0);
        run_instruction(ms, 0x9010);
        ck_assert_int_eq(ms->pc, 2);
        ms->registers[0xA] = 1;
        run_instruction(ms, 0x90A0);
        ck_assert_int_eq(ms->pc, 6);
        ms->registers[0xB] = 0x89;
        ms->registers[0x7] = 0x89;
        run_instruction(ms, 0x9AB0);
        ck_assert_int_eq(ms->pc, 10);
        run_instruction(ms, 0x9B70);
        ck_assert_int_eq(ms->pc, 12);
        delete_mState(&ms);
}
END_TEST

/* Test the load immediate  into to I instruction
 * 0xANNN sets the I registers to a 12 bit const */
START_TEST(test_i_load_immdiate_instruct){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_uint_eq(ms->iRegister, 0);
        run_instruction(ms, 0xABBB);
        ck_assert_uint_eq(ms->iRegister, 0xBBB);
        run_instruction(ms, 0xA000);
        ck_assert_uint_eq(ms->iRegister, 0x000);
        run_instruction(ms, 0xA123);
        ck_assert_uint_eq(ms->iRegister, 0x123);
        run_instruction(ms, 0xAEEE);
        ck_assert_uint_eq(ms->iRegister, 0xEEE);

        delete_mState(&ms);
}
END_TEST

/* Test the jump relative instruction 
 * 0xBNNN jump to V0 + a 12 bit const */
START_TEST(test_jump_relative_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        ck_assert_uint_eq(ms->pc, 0);
        run_instruction(ms, 0xB123);
        ck_assert_uint_eq(ms->pc, 0x123);
        run_instruction(ms, 0xB123);
        ck_assert_uint_eq(ms->pc, 0x123);
        ms->registers[0] = 0x81;
        run_instruction(ms, 0xB123);
        ck_assert_uint_eq(ms->pc, 0x1A4);
        ms->registers[0] = 0xFF;
        run_instruction(ms, 0xB100);
        ck_assert_uint_eq(ms->pc, 0x1FF);

        delete_mState(&ms);
}
END_TEST;

/* Test the rand instruction
 * 0xCXNN generates a random number [0,255] ands it with an 8bin immediate
 * value and stores it in Vx 
 * Vx = (rand() % 256) & NN); */
START_TEST(test_rand_instruction){
        struct mState *ms;
        ms = create_mState();
        ck_assert_ptr_nonnull(ms);
        /* Anding with zero should always generate 0 */
        run_instruction(ms, 0xC000);
        ck_assert_uint_eq(ms->registers[0], 0);

        /* Basic test that random spread is correct */
        size_t sum = 0;
        size_t i;
        for(i = 0; i < 10000; i++){
                run_instruction(ms, 0xC001);
                /* Assert that the value is a one or zero */
                ck_assert_uint_le(ms->registers[0], 1);
                sum += ms->registers[0];
        }
        /* Assert that the number of 1s is within 2% of the expected value of
         * 500 */
        ck_assert_uint_le(sum, 5100);
        ck_assert_uint_ge(sum, 4900);

        /* check the extreme values */
        /* assert that 255 will appear */
        for(i = 0; i < 100000; i++){
                run_instruction(ms, 0xC0FF);
                if(ms->registers[0] == 0xFF)
                        break;
        }
        ck_assert_uint_eq(ms->registers[0], 0xFF);

        /* assert that 0 will appear */
        for(i = 0; i < 100000; i++){
                run_instruction(ms, 0xC0FF);
                if(ms->registers[0] == 0x00)
                        break;
        }
        ck_assert_uint_eq(ms->registers[0], 0x00);

        delete_mState(&ms);
}
END_TEST

Suite *chip8_suite(void){
        Suite *s;
        TCase *tc_core;
        TCase *tc_ins;
        s = suite_create("mState");
        tc_core = tcase_create("core");
        tc_ins = tcase_create("instructions");

        /* Core operations */
        tcase_add_test(tc_core, test_mState_create);
        tcase_add_test(tc_core, test_mState_delete);
        suite_add_tcase(s, tc_core);
        /* Instructions */
        tcase_add_test(tc_ins, test_return_instruction);
        tcase_add_test(tc_ins, test_subroutine_call_instruction);
        tcase_add_test(tc_ins, test_equal_immediate);
        tcase_add_test(tc_ins, test_not_equal_immediate);
        tcase_add_test(tc_ins, test_equal);
        tcase_add_test(tc_ins, test_load_immediate_instruction);
        tcase_add_test(tc_ins, test_add_immediate_instruction);
        tcase_add_test(tc_ins, test_move_instruction);
        tcase_add_test(tc_ins, test_or_instruction);
        tcase_add_test(tc_ins, test_and_instruction);
        tcase_add_test(tc_ins, test_xor_instruction);
        tcase_add_test(tc_ins, test_add_instruction);
        tcase_add_test(tc_ins, test_subtract_instruction);
        tcase_add_test(tc_ins, test_right_shift_instruction);
        tcase_add_test(tc_ins, test_subtract_yx_instruction);
        tcase_add_test(tc_ins, test_left_shift_instruction);
        tcase_add_test(tc_ins, test_not_equal_instruction);
        tcase_add_test(tc_ins, test_i_load_immdiate_instruct);
        tcase_add_test(tc_ins, test_jump_relative_instruction);
        tcase_add_test(tc_ins, test_rand_instruction);
        suite_add_tcase(s, tc_ins);

        return s;
}

Suite *chip8_instructions(void){


}


int main(int argc, char **argv){
        srand(time(NULL));
        Suite *s;
        SRunner *sr;
        int number_failed;

        s = chip8_suite();
        sr = srunner_create(s);
        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);

        srunner_free(sr);

        return (number_failed == 0) ? 1 : 0;

        return 0;
}
