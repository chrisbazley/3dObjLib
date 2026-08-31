/*
 * 3dObjLib test: Vector mathematics
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <stdio.h>

/* 3dObjLib headers */
#include "Vector.h"

/* Local headers */
#include "Tests.h"

static bool same_vector(Coord (*a)[3], Coord x, Coord y, Coord z)
{
  const Coord b[3] = {x, y, z};
  return coord_equal((*a)[0], b[0]) &&
         coord_equal((*a)[1], b[1]) &&
         coord_equal((*a)[2], b[2]);
}

static void test1(void)
{
  /* Plane axes */
  Coord v[3] = {10, 20, 30};
  const Plane p = {2, 0, 1};

  assert(vector_x(&v, p) == &v[2]);
  assert(vector_y(&v, p) == &v[0]);
  assert(vector_z(&v, p) == &v[1]);
}

static void test2(void)
{
  /* Scalar multiply */
  Coord v[3] = {1, -2, 3}, out[3];

  vector_mul(&v, 2, &out);
  assert(same_vector(&out, 2, -4, 6));

  vector_mul(&v, 0, &out);
  assert(same_vector(&out, 0, 0, 0));
}

static void test3(void)
{
  /* Addition and subtraction */
  Coord a[3] = {1, 2, 3}, b[3] = {4, 6, 8}, out[3];

  vector_add(&a, &b, &out);
  assert(same_vector(&out, 5, 8, 11));

  vector_sub(&b, &a, &out);
  assert(same_vector(&out, 3, 4, 5));
}

static void test4(void)
{
  /* Cross and dot products */
  Coord x[3] = {1, 0, 0}, y[3] = {0, 1, 0}, out[3];

  vector_cross(&x, &y, &out);
  assert(same_vector(&out, 0, 0, 1));
  assert(coord_equal(vector_dot(&x, &y), 0));
  assert(coord_equal(vector_dot(&x, &x), 1));
}

static void test5(void)
{
  /* Magnitude and normalization */
  Coord v[3] = {3, 4, 0}, unit[3], zero[3] = {0, 0, 0};

  assert(coord_equal(vector_mag(&v), 5));
  assert(vector_norm(&v, &unit));
  assert(coord_equal(vector_mag(&unit), 1));
  assert(!vector_norm(&zero, &unit));
}

static void test6(void)
{
  /* Approximate vector equality */
  Coord a[3] = {1, 2, 3};
  Coord b[3] = {1 + MAX_FLT_ERR / 2.0, 2, 3};
  Coord c[3] = {1 + 2 * MAX_FLT_ERR, 2, 3};

  assert(vector_equal(&a, &a));
  assert(vector_equal(&a, &b));
  assert(!vector_equal(&a, &c));
}

static void test7(void)
{
  /* Projected rectangle comparisons */
  const Plane p = {0, 1, 2};
  Coord a[3] = {1, 2, 9}, b[3] = {3, 4, -9};
  Coord mixed[3] = {4, 1, 0};

  assert(vector_xy_less_than(&a, &b, p));
  assert(!vector_xy_less_than(&mixed, &b, p));
  assert(vector_xy_greater_or_equal(&b, &a, p));
  assert(!vector_xy_greater_or_equal(&mixed, &a, p));
}

static void test8(void)
{
  /* Line equation */
  const Plane p = {0, 1, 2};
  Coord a[3] = {1, 3, 0}, b[3] = {3, 7, 0};

  const Coord m = vector_y_gradient(&a, &b, p);
  assert(coord_equal(m, 2));
  assert(coord_equal(vector_y_intercept(&a, m, p), 1));
}

static void test9(void)
{
  /* Line intersection */
  const Plane p = {0, 1, 2};
  Coord a[3] = {0, 0, 0}, b[3] = {2, 2, 0};
  Coord c[3] = {0, 2, 0}, d[3] = {2, 0, 0};
  Coord out[3];

  assert(vector_intersect(&a, &b, &c, &d, p, &out));
  assert(same_vector(&out, 1, 1, 0));

  c[0] = 0; c[1] = 1;
  d[0] = 2; d[1] = 3;
  assert(!vector_intersect(&a, &b, &c, &d, p, &out));
}

static void test10(void)
{
  /* Find projection plane */
  Coord v[3] = {1, 10, 2};
  Plane p;

  vector_find_plane(&v, &p);
  assert(p.z == 1);
  assert(p.x < 3);
  assert(p.y < 3);
  assert(p.x != p.y);
  assert(p.x != p.z);
  assert(p.y != p.z);
}

static void test11(void)
{
  /* Debug print interface */
  Coord v[3] = {1, 2, 3};
  vector_print(&v);
  putchar('\n');
}

void Vector_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Plane axes", test1},
    {"Scalar multiply", test2},
    {"Addition and subtraction", test3},
    {"Cross and dot products", test4},
    {"Magnitude and normalization", test5},
    {"Approximate vector equality", test6},
    {"Projected rectangle comparisons", test7},
    {"Line equation", test8},
    {"Line intersection", test9},
    {"Find projection plane", test10},
    {"Debug print interface", test11},
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
