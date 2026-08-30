/*
 * 3dObjLib test: Coordinate data type
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <math.h>
#include <stdio.h>

/* 3dObjLib headers */
#include "Coord.h"

/* Local headers */
#include "Tests.h"

static void test1(void)
{
  /* Absolute value */
  assert(coord_abs(-2.5) == 2.5);
  assert(coord_abs(0.0) == 0.0);
  assert(coord_abs(2.5) == 2.5);
}

static void test2(void)
{
  /* Square root */
  assert(coord_sqrt(0.0) == 0.0);
  assert(coord_sqrt(9.0) == 3.0);
}

static void test3(void)
{
  /* Approximate equality */
  assert(coord_equal(1.0, 1.0));
  assert(coord_equal(1.0, 1.0 + MAX_FLT_ERR / 2.0));
  assert(coord_equal(1.0, 1.0 - MAX_FLT_ERR / 2.0));
  assert(!coord_equal(0.0, MAX_FLT_ERR));
  assert(!coord_equal(0.0, -MAX_FLT_ERR));
}

static void test4(void)
{
  /* Approximate ordering */
  assert(coord_less_than(0.0, MAX_FLT_ERR));
  assert(!coord_less_than(1.0, 1.0 + MAX_FLT_ERR / 2.0));
  assert(!coord_less_than(1.0, 1.0));
  assert(!coord_less_than(2.0, 1.0));
}

static void test5(void)
{
  /* Infinity */
  assert(isinf(COORD_INF));
  assert(COORD_INF > 0);
}

void Coord_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Absolute value", test1},
    {"Square root", test2},
    {"Approximate equality", test3},
    {"Approximate ordering", test4},
    {"Infinity", test5},
  };

  for (size_t count = 0; count < ARRAY_SIZE(unit_tests); count++)
  {
    printf("Test %zu/%zu : %s\n", 1 + count, ARRAY_SIZE(unit_tests),
           unit_tests[count].test_name);
    Fortify_EnterScope();
    unit_tests[count].test_func();
    Fortify_LeaveScope();
  }
}
