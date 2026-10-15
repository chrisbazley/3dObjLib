/* Copyright 2026 Christopher Bazley
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>

unsigned char __at (0x0800) __sdcc_heap[0x1800];
const unsigned int __sdcc_heap_size = sizeof(__sdcc_heap);
volatile unsigned char __at (0xfff0) simif;

int putchar(int c)
{
  simif = 'w';
  simif = (unsigned char)c;
  return (unsigned char)c;
}

_Noreturn static void stop(void)
{
  simif = 's';
  for (;;) {}
}

_Noreturn void abort(void)
{
  puts("3DOBJ TEST ABORT");
  stop();
}

_Noreturn void cbutil_sdcc_assert(unsigned int line)
{
  puts("ASSERT FAIL");
  putchar('0' + line / 1000);
  putchar('0' + line / 100 % 10);
  putchar('0' + line / 10 % 10);
  putchar('0' + line % 10);
  putchar('\n');
  stop();
}

void TEST_FUNCTION(void);
void __sdcc_heap_init(void);

int main(void)
{
  __sdcc_heap_init();
  TEST_FUNCTION();
  puts("3DOBJ TEST PASS");
  stop();
}
