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
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "chip8_test.h"

#include "../src/chip8.h"

static struct mState *ms;

void chip8_setup(void){
        ms = chip8_init();
        ck_assert_ptr_nonnull(ms);
}

void chip8_teardown(void){
        chip8_destroy(&ms);
        ck_assert_ptr_null(ms);
}

START_TEST(test_chip8_init){
        struct mState *ms;
        ms = chip8_init();
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->stackSize, 0);
        ck_assert_int_eq(ms->pc, 0x200);
        ck_assert_int_eq(ms->iRegister, 0);
        for(int i = 0; i < 16; i++)
                ck_assert_int_eq(ms->registers[1], 0);
        chip8_destroy(&ms);
}
END_TEST

START_TEST(test_chip8_destroy){
        struct mState *ms;
        ms = chip8_init();
        ck_assert_ptr_nonnull(ms);
        chip8_destroy(&ms);
        ck_assert_ptr_null(ms);
}
END_TEST

START_TEST(test_chip8_load_rom){
        struct runtime_error *re;
        re = chip8_load_rom(ms, "testdata/nonexistence");
        ck_assert_ptr_nonnull(re);
        ck_assert_str_eq(re->msg, "Could not open ROM file: \"testdata/nonexistence\"");
        re = chip8_load_rom(ms, "testdata/invalid");
        ck_assert_ptr_nonnull(re);
        ck_assert_str_eq(re->msg, "ROM file, \"testdata/invalid\", is 4096 bytes which is more than the max ROM size of 3584 bytes");
}
END_TEST

START_TEST(test_chip8_run){
        /* Set V0 to BB */
        ms->mem[0x400] = 0x60;
        ms->mem[0x401] = 0xBB;
        /* Set Vf to BC */
        ms->mem[0x402] = 0x6F;
        ms->mem[0x403] = 0xBC;
        /* loop */
        ms->mem[0x404] = 0x14;
        ms->mem[0x405] = 0x04;
        
        ms->pc = 0x400;
        chip8_run(ms);

        sleep(4);

        chip8_halt(ms);

        ck_assert_uint_eq(ms->registers[0x0], 0xBB);
        ck_assert_uint_eq(ms->registers[0xf], 0xBc);
        ck_assert_uint_eq(ms->pc, 0x404);
        //printf("%lu instructions in 4 seconds. %f instructions per a second\n", ms->count, ((double) ms->count) / 4.0);
}
END_TEST

START_TEST(test_wait_for_keypress_instruction){
        struct keyEvent ke;
        ms->mem[0x400] = 0xF1;
        ms->mem[0x401] = 0x0A;
        ms->mem[0x402] = 0x14;
        ms->mem[0x403] = 0x02;
        ms->pc = 0x400;

        ke.type = Pressed;
        ke.key  = 0xB;
        chip8_run(ms);

        sleep(1);

        chip8_key_event_notify(ms, ke);

        sleep(1);

        chip8_halt(ms);

        ck_assert_uint_eq(ms->registers[0x1], 0xB);
        ck_assert_uint_eq(ms->pc, 0x402);
        ck_assert_uint_eq(ms->keys[0xB], 1);

        ms->mem[0x400] = 0xF1;
        ms->mem[0x401] = 0x0A;
        ms->mem[0x402] = 0x14;
        ms->mem[0x403] = 0x02;
        ms->pc = 0x400;

        ke.type = Released;
        ke.key  = 0x1;

        chip8_run(ms);

        sleep(1);

        chip8_key_event_notify(ms, ke);

        sleep(1);

        chip8_halt(ms);

        ck_assert_uint_ne(ms->registers[0x1], 0x1);
        ck_assert_uint_eq(ms->pc, 0x402);
        ck_assert_uint_eq(ms->keys[0x1], 0);
}
END_TEST

START_TEST(test_chip8_timers){
        ms->registers[0x0] = 0xFF;
        ms->mem[0x400] = 0xF0;
        ms->mem[0x401] = 0x15;
        ms->mem[0x402] = 0xF0;
        ms->mem[0x403] = 0x18;
        ms->mem[0x404] = 0x14;
        ms->mem[0x405] = 0x04;

        ms->pc = 0x400;

        chip8_run(ms);

        sleep(5);

        chip8_halt(ms);

        ck_assert_uint_eq(ms->sTimer, 0);
        ck_assert_uint_eq(ms->dTimer, 0);

        /* Set dTimer to 250 */
        ms->registers[0x0] = 0xFF;
        ms->mem[0x400] = 0xF0;
        ms->mem[0x401] = 0x15;
        /* Set sTimer to 250 */
        ms->mem[0x402] = 0xF0;
        ms->mem[0x403] = 0x18;
        /* loop */
        ms->mem[0x404] = 0x14;
        ms->mem[0x405] = 0x04;
        
        ms->pc = 0x400;
       
        uint8_t sSTime, sDTime, eSTime, eDTime;
        struct timespec ts, ts2;
        ts.tv_nsec = 0;
        ts.tv_sec = 1;

        /* start the chip */
        chip8_run(ms);

        nanosleep(&ts, &ts2);
        sSTime = ms->sTimer;
        sDTime = ms->dTimer;
        nanosleep(&ts, &ts2);
        eSTime = ms->sTimer;
        eDTime = ms->dTimer;

        chip8_halt(ms);
        ck_assert_uint_eq(sSTime - 60, eSTime);
        ck_assert_uint_eq(sDTime - 60, eDTime);
}
END_TEST

/* Test clear display instruction:
 * 0x0E0 clears the display */
START_TEST(test_clear_instruction){
        run_instruction(ms, 0x00E0);
        for(size_t y = 0; y < 32; y++)
                for(size_t x = 0; x < 8; x++)
                        ck_assert_uint_eq(ms->disp[y][x], 0x0);
        for(size_t y = 0; y < 32; y++)
                for(size_t x = 0; x < 8; x++)
                        ms->disp[x][y] = 1;
        run_instruction(ms, 0x00E0);
        for(int x = 0; x < 8; x++)
                for(int y = 0; y < 4; y++)
                        ck_assert_uint_eq(ms->disp[x][y], 0x0);
}
END_TEST

START_TEST(test_return_instruction){
        ms->stack[0] = 0x1212;
        ms->stackSize = 1;
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x00EE);
        ck_assert_int_eq(ms->pc, 0x1212);
        ck_assert_int_eq(ms->stackSize, 0);
        
}
END_TEST

/* Test Jump instruction:
 * 0x1NNN jump to 12 bit immediate value */
START_TEST(test_jump_instruction){
        ck_assert_ptr_nonnull(ms);
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x1456);
        ck_assert_int_eq(ms->pc, 0x456);
        run_instruction(ms, 0x1956);
        ck_assert_int_eq(ms->pc, 0x956);
        run_instruction(ms, 0x1256);
        ck_assert_int_eq(ms->pc, 0x256);
}
END_TEST

START_TEST(test_subroutine_call_instruction){
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x2523);
        ck_assert_int_eq(ms->pc, 0x523);
        ck_assert_int_eq(ms->stackSize, 1);
        ck_assert_int_eq(ms->stack[0], 0x202);
}
END_TEST

START_TEST(test_equal_immediate){
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x3000);
        ck_assert_int_eq(ms->pc, 0x204);
        run_instruction(ms, 0x3034);
        ck_assert_int_eq(ms->pc, 0x206);
        ms->registers[7] = 0x82;
        run_instruction(ms, 0x3782);
        ck_assert_uint_eq(ms->pc, 0x20A);
        run_instruction(ms, 0x3772);
        ck_assert_uint_eq(ms->pc, 0x20C);
}
END_TEST

START_TEST(test_not_equal_immediate){
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x4000);
        ck_assert_int_eq(ms->pc, 0x202);
        run_instruction(ms, 0x4001);
        ck_assert_int_eq(ms->pc, 0x206);
        ms->registers[8] = 0x89;
        run_instruction(ms, 0x4889);
        ck_assert_int_eq(ms->pc, 0x208);
        run_instruction(ms, 0x4890);
        ck_assert_int_eq(ms->pc, 0x20C);
}
END_TEST

START_TEST(test_equal){
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x5010);
        ck_assert_int_eq(ms->pc, 0x204);
        ms->registers[0xA] = 1;
        run_instruction(ms, 0x50A0);
        ck_assert_int_eq(ms->pc, 0x206);
        ms->registers[0xB] = 0x89;
        ms->registers[0x7] = 0x89;
        run_instruction(ms, 0x5AB0);
        ck_assert_int_eq(ms->pc, 0x208);
        run_instruction(ms, 0x5B70);
        ck_assert_int_eq(ms->pc, 0x20C);
}
END_TEST

START_TEST(test_load_immediate_instruction){
        run_instruction(ms, 0x6AFF);
        ck_assert_uint_eq(ms->registers[0xA], 0xFF);
        run_instruction(ms, 0x6A11);
        ck_assert_uint_eq(ms->registers[0xA], 0x11);
        run_instruction(ms, 0x6111);
        ck_assert_uint_eq(ms->registers[0x1], 0x11);
        run_instruction(ms, 0x6099);
        ck_assert_uint_eq(ms->registers[0x0], 0x99);
}
END_TEST

START_TEST(test_add_immediate_instruction){
        run_instruction(ms, 0x7011);
        ck_assert_uint_eq(ms->registers[0x0], 0x11);
        run_instruction(ms, 0x7011);
        ck_assert_uint_eq(ms->registers[0x0], 0x22);
        run_instruction(ms, 0x7A13);
        ck_assert_uint_eq(ms->registers[0xA], 0x13);
        run_instruction(ms, 0x7A0F);
        ck_assert_uint_eq(ms->registers[0xA], 0x22);
}
END_TEST

START_TEST(test_move_instruction){
        ms->registers[0x0] = 0x99;
        run_instruction(ms, 0x8A00);
        ck_assert_uint_eq(ms->registers[0xA], 0x99);
        run_instruction(ms, 0x8010);
        ck_assert_uint_eq(ms->registers[0x0], 0x00);
        run_instruction(ms, 0x8DA0);
        ck_assert_uint_eq(ms->registers[0xD], 0x99);
        run_instruction(ms, 0x8A00);
        ck_assert_uint_eq(ms->registers[0xA], 0x00);
}
END_TEST

START_TEST(test_or_instruction){
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
}
END_TEST

START_TEST(test_and_instruction){
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
}
END_TEST

START_TEST(test_xor_instruction){
        ms->registers[0x0] = 0x11;
        run_instruction(ms, 0x8103);
        ck_assert_uint_eq(ms->registers[0x1], 0x11);
        ms->registers[0x2] = 0x11;
        ms->registers[0x3] = 0x11;
        run_instruction(ms, 0x8323);
        ck_assert_uint_eq(ms->registers[0x3], 0x00);
}
END_TEST

START_TEST(test_add_instruction){
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
}
END_TEST

/* Test the 0x8XY5 instruction */
START_TEST(test_subtract_instruction){
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
}
END_TEST

/* Test the right shift instruction
 * 0x8XY6 shifts Vy to the right by 1, assign the value to Vx and sets Vf to the least
 * significant bit of Vy from before the shift */ 
START_TEST(test_right_shift_instruction){
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
}
END_TEST

/* Test the Vx = Vy - Vx instruction
 * 0x8XY7 sets Vx to Vy - Vx and sets Vf to 1 if there is a carry or to 0 if
 * there is not carry */
START_TEST(test_subtract_yx_instruction){
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
}
END_TEST

/* Test the left shift instruction
 * 0x8XYE shifts Vy  to the left by 1, assign the value to Vx and sets Vf to the
 * most significant bit of Vy from before the shift */ 
START_TEST(test_left_shift_instruction){
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
}
END_TEST

/* Test the not equal operator
 * 0x9XY0 skips the next instruct if Vx != Vy */
START_TEST(test_not_equal_instruction){
        ck_assert_int_eq(ms->pc, 0x200);
        run_instruction(ms, 0x9010);
        ck_assert_int_eq(ms->pc, 0x202);
        ms->registers[0xA] = 1;
        run_instruction(ms, 0x90A0);
        ck_assert_int_eq(ms->pc, 0x206);
        ms->registers[0xB] = 0x89;
        ms->registers[0x7] = 0x89;
        run_instruction(ms, 0x9AB0);
        ck_assert_int_eq(ms->pc, 0x20A);
        run_instruction(ms, 0x9B70);
        ck_assert_int_eq(ms->pc, 0x20C);
}
END_TEST

/* Test the load immediate  into to I instruction
 * 0xANNN sets the I registers to a 12 bit const */
START_TEST(test_i_load_immdiate_instruct){
        ck_assert_uint_eq(ms->iRegister, 0);
        run_instruction(ms, 0xABBB);
        ck_assert_uint_eq(ms->iRegister, 0xBBB);
        run_instruction(ms, 0xA000);
        ck_assert_uint_eq(ms->iRegister, 0x000);
        run_instruction(ms, 0xA123);
        ck_assert_uint_eq(ms->iRegister, 0x123);
        run_instruction(ms, 0xAEEE);
        ck_assert_uint_eq(ms->iRegister, 0xEEE);
}
END_TEST

/* Test the jump relative instruction 
 * 0xBNNN jump to V0 + a 12 bit const */
START_TEST(test_jump_relative_instruction){
        ck_assert_uint_eq(ms->pc, 0x200);
        run_instruction(ms, 0xB422);
        ck_assert_uint_eq(ms->pc, 0x422);
        ms->registers[0] = 0x82;
        run_instruction(ms, 0xB400);
        ck_assert_uint_eq(ms->pc, 0x482);
        ms->registers[0] = 0xFF;
        run_instruction(ms, 0xB300);
        ck_assert_uint_eq(ms->pc, 0x3FF);
}
END_TEST;

/* Test the rand instruction
 * 0xCXNN generates a random number [0,255] ands it with an 8bin immediate
 * value and stores it in Vx 
 * Vx = (rand() % 256) & NN); */
START_TEST(test_rand_instruction){
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
        /* Assert that the number of 1s is within 4% of the expected value of
         * 500 
         * This should likely never fail if it does it should pass the next
         * time*/
        ck_assert_uint_le(sum, 5200);
        ck_assert_uint_ge(sum, 4800);

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
}
END_TEST

/* Test the draw instruction 
 * 0xDXYN draws an 8 by N sprite at (X, Y), the pixles in the sprite are xor'd
 * with the pixels on the screen. If any pixels go from 1 to 0 Vf is  set to 1.
 * Otherwise Vf is set to 0.
 */
START_TEST(test_draw_instruction){
        /* check that one line is drawn correctly */
        ms->mem[0] = 0b11111111;
        ms->registers[0] = 0x0;
        ms->registers[1] = 0x0;
        ms->iRegister = 0;
        run_instruction(ms, 0xD011);
        ck_assert_uint_eq(ms->disp[0][0], 0b11111111);
        ck_assert_uint_eq(ms->registers[0xF], 0);

        /* check that two lines are drawn correctly */
        ms->mem[1] = 0b10101010;
        ms->mem[2] = 0b10101010;
        ms->iRegister = 1;
        ms->registers[0] = 0;
        ms->registers[1] = 1;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[1][0], 0b10101010);
        ck_assert_uint_eq(ms->disp[2][0], 0b10101010);
        ck_assert_uint_eq(ms->registers[0xF], 0);

        /* check that pixels are XOR'd */
        ms->mem[1] = 0b10101010;
        ms->mem[2] = 0b10101010;
        ms->iRegister = 1;
        ms->registers[0] = 0;
        ms->registers[1] = 1;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[1][0], 0b00000000);
        ck_assert_uint_eq(ms->disp[2][0], 0b00000000);
        ck_assert_uint_eq(ms->registers[0xF], 1);

        /* test sprites that overlap byte bounds */
        ms->mem[3] = 0b11111111;
        ms->mem[4] = 0b11111111;
        ms->iRegister = 3;
        ms->registers[0] = 4;
        ms->registers[1] = 4;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[4][0], 0b00001111);
        ck_assert_uint_eq(ms->disp[4][1], 0b11110000);
        ck_assert_uint_eq(ms->disp[5][0], 0b00001111);
        ck_assert_uint_eq(ms->disp[5][1], 0b11110000);
        ck_assert_uint_eq(ms->registers[0xF], 0);

        /* test line wraping works */
        ms->mem[0x0EE] = 0b11111111;
        ms->mem[0x0EF] = 0b11111111;
        ms->iRegister = 0x0EE;
        ms->registers[0] = 60;
        ms->registers[1] = 8;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[8][7], 0b00001111);
        ck_assert_uint_eq(ms->disp[8][0], 0b11110000);
        ck_assert_uint_eq(ms->disp[9][7], 0b00001111);
        ck_assert_uint_eq(ms->disp[9][0], 0b11110000);
        ck_assert_uint_eq(ms->registers[0xF], 0);

        /* test that x and y values out side the screen are wrapped to fit */
        ms->mem[0x099] = 0b11001100;
        ms->mem[0x09A] = 0b11001100;
        ms->iRegister = 0x099;
        /* X should become 130 - (64 * 2) = 2 */
        ms->registers[0] = 130;
        /* Y should become 74 - (32 * 2) = 10 */
        ms->registers[1] = 74;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[10][0], 0b00110011);
        ck_assert_uint_eq(ms->disp[10][1], 0b00000000);
        ck_assert_uint_eq(ms->disp[11][0], 0b00110011);
        ck_assert_uint_eq(ms->disp[11][1], 0b00000000);
        ck_assert_uint_eq(ms->registers[0xF], 0);

        /* test sprites that overlap byte bounds with odd number overlap */
        ms->mem[3] = 0b11111111;
        ms->mem[4] = 0b11111111;
        ms->iRegister = 3;
        ms->registers[0] = 7;
        ms->registers[1] = 28;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[28][0], 0b00000001);
        ck_assert_uint_eq(ms->disp[28][1], 0b11111110);
        ck_assert_uint_eq(ms->disp[29][0], 0b00000001);
        ck_assert_uint_eq(ms->disp[29][1], 0b11111110);
        ck_assert_uint_eq(ms->registers[0xF], 0);
        
        /* clear the display */
        run_instruction(ms, 0x00E0);

        /* test row wrap around */
        ms->mem[3] = 0b11111111;
        ms->mem[4] = 0b11111111;
        ms->iRegister = 3;
        ms->registers[0] = 7;
        ms->registers[1] = 31;
        run_instruction(ms, 0xD012);
        ck_assert_uint_eq(ms->disp[31][0], 0b00000001);
        ck_assert_uint_eq(ms->disp[31][1], 0b11111110);
        ck_assert_uint_eq(ms->disp[0][0], 0b00000001);
        ck_assert_uint_eq(ms->disp[0][1], 0b11111110);
        ck_assert_uint_eq(ms->registers[0xF], 0);
}
END_TEST

/* Test the is key press instruction 
 * 0xEX9E check if the keycode in X is currently pressed. If the key is pressed,
 * the next instructions is skipped */
START_TEST(test_key_press_instruction){
        ck_assert_int_eq(ms->pc, 0x200);

        ms->keys[0xF] = 1;
        ms->registers[0x0] = 0xF;
        run_instruction(ms, 0xE09E);
        ck_assert_int_eq(ms->pc, 0x204);

        ms->registers[0xA] = 0;
        run_instruction(ms, 0xEA9E);
        ck_assert_int_eq(ms->pc, 0x206);

        ms->keys[0xB] = 1;
        ms->registers[0x7] = 0xC;
        run_instruction(ms, 0xE79E);
        ck_assert_int_eq(ms->pc, 0x208);

        ms->keys[0xC] = 1;
        run_instruction(ms, 0xE79E);
        ck_assert_int_eq(ms->pc, 0x20C);
}
END_TEST;

/* Test the is key not press instruction 
 * 0xEX9E check if the keycode in X is currently pressed. If the key is not  pressed,
 * the next instructions is skipped */
START_TEST(test_key_not_press_instruction){
        ck_assert_int_eq(ms->pc, 0x200);

        ms->keys[0xF] = 1;
        ms->registers[0] = 0xF;
        run_instruction(ms, 0xE0A1);
        ck_assert_int_eq(ms->pc, 0x202);

        ms->registers[0xA] = 0;
        run_instruction(ms, 0xEAA1);
        ck_assert_int_eq(ms->pc, 0x206);

        ms->keys[0xB] = 1;
        ms->registers[0x7] = 0xC;
        run_instruction(ms, 0xE7A1);
        ck_assert_int_eq(ms->pc, 0x20A);

        ms->keys[0xC] = 1;
        run_instruction(ms, 0xE7A1);
        ck_assert_int_eq(ms->pc, 0x20C);
}
END_TEST;

/* Test the get delay timer instruction
 * 0xFX07 should set Vx to the delay timer's currenty value */
START_TEST(test_get_timer_instruction){
        ms->dTimer = 0xBC;
        run_instruction(ms, 0xF007);
        ck_assert_uint_eq(ms->registers[0x0], 0xBC);

        ms->dTimer = 0x9C;
        run_instruction(ms, 0xF107);
        ck_assert_uint_eq(ms->registers[0x1], 0x9C);

        ms->dTimer = 0x00;
        run_instruction(ms, 0xFA07);
        ck_assert_uint_eq(ms->registers[0xA], 0x00);

        ms->dTimer = 0xFF;
        run_instruction(ms, 0xFF07);
        ck_assert_uint_eq(ms->registers[0xF], 0xFF);
}
END_TEST

/* Test the set delay timer instruction 
 * 0xFX15 should the delay timer to the value in Vx */
START_TEST(test_set_delay_timer_instruction){
        ms->registers[0x0] = 0xBC;
        run_instruction(ms, 0xF015);
        ck_assert_uint_eq(ms->dTimer, 0xBC);

        ms->registers[0x2] = 0x12;
        run_instruction(ms, 0xF215);
        ck_assert_uint_eq(ms->dTimer, 0x12);

        ms->registers[0xF] = 0x99;
        run_instruction(ms, 0xFF15);
        ck_assert_uint_eq(ms->dTimer, 0x99);

        ms->registers[0x5] = 0x88;
        run_instruction(ms, 0xF515);
        ck_assert_uint_eq(ms->dTimer, 0x88);
}
END_TEST

/* Test the set sound timer instruction 
 * 0xFX18 should the sound timer to the value in Vx */
START_TEST(test_set_sound_timer_instruction){
        ms->registers[0x0] = 0xBC;
        run_instruction(ms, 0xF018);
        ck_assert_uint_eq(ms->sTimer, 0xBC);

        ms->registers[0x2] = 0x12;
        run_instruction(ms, 0xF218);
        ck_assert_uint_eq(ms->sTimer, 0x12);

        ms->registers[0xF] = 0x99;
        run_instruction(ms, 0xFF18);
        ck_assert_uint_eq(ms->sTimer, 0x99);

        ms->registers[0x5] = 0x88;
        run_instruction(ms, 0xF518);
        ck_assert_uint_eq(ms->sTimer, 0x88);
}
END_TEST

/* Test the add to i register instruction 
 * 0xFX1E i += Vx */
START_TEST(test_add_i_register_instruction){
        ms->iRegister = 0;
        ms->registers[0x0] = 0x12;
        run_instruction(ms, 0xF01E);
        ck_assert_uint_eq(ms->iRegister, 0x12);

        ms->registers[0xB] = 0x90;
        run_instruction(ms, 0xFB1E);
        ck_assert_uint_eq(ms->iRegister, 0xA2);
}
END_TEST

/* Test the get font instruction 
 * 0xFX29 stores the address of the 4 bit character in Vx to the I register */
START_TEST(test_get_font_instruction){
        ms->registers[0x0] = 0x00;
        run_instruction(ms, 0xF029);
        ck_assert_uint_eq(ms->mem[ms->iRegister],     0b11110000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 1], 0b10010000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 2], 0b10010000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 3], 0b10010000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 4], 0b11110000);

        ms->registers[0xC] = 0x0F;
        run_instruction(ms, 0xFC29);
        ck_assert_uint_eq(ms->mem[ms->iRegister],     0b11110000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 1], 0b10000000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 2], 0b11110000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 3], 0b10000000);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 4], 0b10000000);
}
END_TEST

/* Test the to BCD instruction
 * 0xFX33 should store the BCD rep of Vx starting at I address */
START_TEST(test_to_bcd_instruction){
        ms->registers[0x0] = 0x00;
        ms->iRegister = 0;
        run_instruction(ms, 0xF033);
        ck_assert_uint_eq(ms->mem[ms->iRegister],     0);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 1], 0);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 2], 0);

        ms->registers[0xB] = 255;
        ms->iRegister = 0x300;
        run_instruction(ms, 0xFB33);
        ck_assert_uint_eq(ms->mem[ms->iRegister],     2);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 1], 5);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 2], 5);
        
        ms->registers[0xB] = 99;
        ms->iRegister = 0x200;
        run_instruction(ms, 0xFB33);
        ck_assert_uint_eq(ms->mem[ms->iRegister],     0);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 1], 9);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 2], 9);
        
        ms->registers[0xB] = 1;
        ms->iRegister = 0x250;
        run_instruction(ms, 0xFB33);
        ck_assert_uint_eq(ms->mem[ms->iRegister],     0);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 1], 0);
        ck_assert_uint_eq(ms->mem[ms->iRegister + 2], 1);
}
END_TEST

/* Test the dump register instruction
 * 0xFX55 writes [V0, Vx] to memory starting at I. I is increased 1 for each
 * value written */
START_TEST(test_dump_instruction){
        ms->iRegister = 0x400;
        for(size_t i = 0; i < 16; i++)
                ms->registers[i] = rand();
        run_instruction(ms, 0xFF55);
        for(size_t i = 0; i < 16; i++)
                ck_assert_uint_eq(ms->mem[0x400 + i], ms->registers[i]);
        ck_assert_uint_eq(ms->iRegister, 0x410);
        ms->mem[0x501] = 0;
        ms->iRegister = 0x500;
        run_instruction(ms, 0xF055);
        ck_assert_uint_eq(ms->mem[0x500], ms->registers[0]);
        ck_assert_uint_eq(ms->mem[0x501], 0);
        ck_assert_uint_eq(ms->iRegister, 0x501);
}
END_TEST

START_TEST(test_load_instruction){
        ms->iRegister = 0x400;

        for(size_t i = 0; i < 16; i++)
                ms->mem[0x400 + i] = rand();
        run_instruction(ms, 0xFF65);
        for(size_t i = 0; i < 16; i++)
                ck_assert_uint_eq(ms->mem[0x400 + i], ms->registers[i]);
        ck_assert_uint_eq(ms->iRegister, 0x410);

        for(size_t i = 0; i < 3; i++)
                ms->mem[0xBCD + i] = rand();
        ms->iRegister = 0xBCD;
        run_instruction(ms, 0xF265);
        for(size_t i = 0; i < 3; i++)
                ck_assert_uint_eq(ms->mem[0xBCD + i], ms->registers[i]);

        ck_assert_uint_eq(ms->iRegister, 0xBD0);
}
END_TEST


Suite *chip8_suite(void){
        Suite *s;
        TCase *tc_core;
        TCase *tc_ins;
        TCase *tc_func;
        s = suite_create("CHIP 8 Unit Tests");
        tc_core = tcase_create("core");
        tc_ins = tcase_create("instructions");
        tc_func = tcase_create("functionality");

        /* Core operations */
        tcase_add_test(tc_core, test_chip8_init);
        tcase_add_test(tc_core, test_chip8_destroy);
        suite_add_tcase(s, tc_core);
        /* Instructions */
        tcase_add_test(tc_ins, test_clear_instruction);
        tcase_add_test(tc_ins, test_return_instruction);
        tcase_add_test(tc_ins, test_jump_instruction);
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
        tcase_add_test(tc_ins, test_draw_instruction);
        tcase_add_test(tc_ins, test_key_press_instruction);
        tcase_add_test(tc_ins, test_key_not_press_instruction);
        tcase_add_test(tc_ins, test_get_timer_instruction);
        tcase_add_test(tc_ins, test_wait_for_keypress_instruction);
        tcase_add_test(tc_ins, test_set_delay_timer_instruction);
        tcase_add_test(tc_ins, test_set_sound_timer_instruction);
        tcase_add_test(tc_ins, test_add_i_register_instruction);
        tcase_add_test(tc_ins, test_get_font_instruction);
        tcase_add_test(tc_ins, test_to_bcd_instruction);
        tcase_add_test(tc_ins, test_dump_instruction);
        tcase_add_test(tc_ins, test_load_instruction);
        tcase_add_checked_fixture(tc_ins, chip8_setup, chip8_teardown);
        tcase_set_timeout(tc_ins, 10);
        suite_add_tcase(s, tc_ins);

        tcase_add_test(tc_func, test_chip8_run);
        tcase_add_test(tc_func, test_chip8_timers);
        tcase_add_test(tc_func, test_chip8_load_rom);
        tcase_add_checked_fixture(tc_func, chip8_setup, chip8_teardown);
        tcase_set_timeout(tc_func, 20);
        suite_add_tcase(s, tc_func);

        return s;
}


