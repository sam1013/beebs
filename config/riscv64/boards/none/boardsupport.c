/* Copyright (C) 2014 Embecosm Limited and University of Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>

   This file is part of the Bristol/Embecosm Embedded Benchmark Suite.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

#include <support.h>
#include <encoding.h>
#include <stdint.h>
#include <stddef.h>

extern int main();


__attribute__((section(".init")))
void _start()
{
  asm volatile(
  "_start:\n"
    "la		t0, trap_entry\n"
    "csrw	mtvec, t0\n"
    "li		t0, 0x1800\n"	// MPP = 3
    "csrw	mstatus, t0\n"

    /* enable counter */
    "csrsi	mcounteren, 0x7\n"
    "csrsi	scounteren, 0x7\n"

    /* enable floating point */
    "li    t0, 0x2000\n"
    "csrs mstatus, t0\n"

    "li	x1, 0\n"
    "li	x2, 0\n"
    "li	x3, 0\n"
    "li	x4, 0\n"
    "li	x5, 0\n"
    "li	x6, 0\n"
    "li	x7, 0\n"
    "li	x8, 0\n"
    "li	x9, 0\n"
    "li	x10, 0\n"
    "li	x11, 0\n"
    "li	x12, 0\n"
    "li	x13, 0\n"
    "li	x14, 0\n"
    "li	x15, 0\n"
    "li	x16, 0\n"
    "li	x17, 0\n"
    "li	x18, 0\n"
    "li	x19, 0\n"
    "li	x20, 0\n"
    "li	x21, 0\n"
    "li	x22, 0\n"
    "li	x23, 0\n"
    "li	x24, 0\n"
    "li	x25, 0\n"
    "li	x26, 0\n"
    "li	x27, 0\n"
    "li	x28, 0\n"
    "li	x29, 0\n"
    "li	x30, 0\n"
    "li	x31, 0\n"

    /* initialize global pointer */
    "la	gp, _gp\n"

"init_bss:\n"
    /* init bss section */
    "la	a0, __bss_start\n"
    "la	a1, __bss_end\n"
    "li	a2, 0x0\n"
    "jal	fill_block\n"

"init_sbss:\n"
    /* init bss section */
    "la	a0, __sbss_start\n"
    "la	a1, __sbss_end\n"
    "li	a2, 0x0\n"
    "jal	fill_block\n"

"write_stack_pattern:\n"
    /* init bss section */
    "la	a0, _stack_end\n"  /* note the stack grows from top to bottom */
    "la	a1, __stack\n"
    "li	a2, 0xABABABAB\n"
    "jal	fill_block\n"

"init_stack:\n"
    /* set stack pointer */
    "la	sp, _stack\n"
    "jal _start_main\n"

/* Fills memory blocks            */
/* a0 ... start address           */
/* a1 ... end address (exclusive) */
/* a2 ... word pattern to write   */
"fill_block:\n"
    "bgeu	a0, a1, fb_end\n"
    "sw		a2, 0(a0)\n"
    "addi	a0, a0, 4\n"
    "j		fill_block\n"
"fb_end:\n"
    "ret\n"
"trap_entry:\n"
    "j .\n"
    "li a0, -1\n"
    "jal sysExit\n"
    "j .\n"
    );
}

static uint64_t zeroExtend(long val)
{
	uint64_t ret = val;
	#if __riscv_xlen == 32
		ret = (0x00000000ffffffff & val);
	#endif
	return ret;
}

/* HTIF  interface */
volatile uint64_t tohost __attribute__((aligned(64)));
volatile uint64_t fromhost __attribute__((aligned(64)));

/* SPIKE HTIF exit systemcall */
void sysExit(long code)
{
	uint64_t zcode = zeroExtend(code);
	tohost = ((zcode << 1) | 1);
	for(;;) { }
}

void _start_main()
{
  sysExit(main());
}

void initialise_board()
{
}

void start_trigger()
{
#if TM_BENCH == 1
  set_csr(trace, 1);
#endif
}

void stop_trigger()
{
#if TM_BENCH == 1
  set_csr(trace, 2);
#endif
}

/* GCC -ffreestanding still requires memcpy, memmove, memset and memcmp, 
 * so implement them. We ignore memmove for now.
 * See https://cs107e.github.io/guides/gcc/
 */
void* memcpy(void* dest, const void* src, size_t len)
{
  const char* s = src;
  char *d = dest;

  if ((((uintptr_t)dest | (uintptr_t)src) & (sizeof(uintptr_t)-1)) == 0) {
    while (d < ((char*)dest + len - (sizeof(uintptr_t)-1))) {
      *(uintptr_t*)d = *(const uintptr_t*)s;
      d += sizeof(uintptr_t);
      s += sizeof(uintptr_t);
    }
  }

  while (d < ((char*)dest + len))
    *d++ = *s++;

  return dest;
}

void* memset(void* dest, int byte, size_t len)
{
  if ((((uintptr_t)dest | len) & (sizeof(uintptr_t)-1)) == 0) {
    uintptr_t word = byte & 0xFF;
    word |= word << 8;
    word |= word << 16;
    word |= word << 16 << 16;

    uintptr_t *d = dest;
    while (d < (uintptr_t*)((char*)dest + len))
      *d++ = word;
  } else {
    char *d = dest;
    while (d < ((char*)dest + len))
      *d++ = byte;
  }
  return dest;
}

int memcmp(const void* s1, const void* s2, size_t len)
{
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;
  long int val;

  if ((((const uintptr_t)s1 | (const uintptr_t)s2) & (sizeof(uintptr_t)-1)) == 0) {
    while (p1 < ((const unsigned char*)s1 + len - (sizeof(uintptr_t)-1))) {
      val = *(const uintptr_t*)p1 - *(const uintptr_t*)p2;
      if (val != 0) {
        return val > 0 ? 1 : -1;
      }
      p1 += sizeof(uintptr_t);
      p2 += sizeof(uintptr_t);
    }
  }

  while (p1 < ((const unsigned char*)s1 + len)) {
    val = *p1 - *p2;
    if (val != 0) {
      return val > 0 ? 1 : -1;
    }
    p1++;
    p2++;
  }
  return 0;
}

#include "heapwrappers.c"

size_t strlen(const char *s) {
  size_t len = 0;
  while(*s++) {
    len++;
  }
  return len;
}

unsigned long int rand_next = 1;

/*
 *  int rand()
 *      Taken from the K&R C programming language book. Page 46.
 *      returns a pseudo-random integer of 0..32767. Note that
 *      this is compatible with the System V function rand(), not
 *      with the bsd function rand() that returns 0..(2**31)-1.
 */
int 
rand ()
{
	rand_next = rand_next * 1103515245 + 12345;
	return ((unsigned int)(rand_next / 65536) % 32768);
}

/*
 *  srand(seed)
 *      companion routine to rand(). Initializes the seed.
 */
void
srand(unsigned int seed)
{
	rand_next = seed;
}
