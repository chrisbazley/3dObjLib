/*
 * 3dObjLib test: Primitive groups
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* 3dObjLib headers */
#include "Group.h"

/* Local headers */
#include "Tests.h"

static void test1(void)
{
  /* Init, delete all and free */
  Group group;
  memset(&group, CHAR_MAX, sizeof(group));

  group_init(&group);
  assert(group_get_num_primitives(&group) == 0);
  assert(group_get_primitive(&group, 0) == NULL);

  assert(group_add_primitive(&group) != NULL);
  assert(group_get_num_primitives(&group) == 1);

  group_delete_all(&group);
  assert(group_get_num_primitives(&group) == 0);
  group_free(&group);
}

static void test2(void)
{
  /* Allocation */
  Group group;
  group_init(&group);

  assert(group_alloc_primitives(&group, -1) == 0);
  assert(group_alloc_primitives(&group, 1) >= 1);
  {
    const int old_alloc = group.nalloc;
    assert(group_alloc_primitives(&group, old_alloc + 1) > old_alloc);
  }

  group_free(&group);
}

static void test3(void)
{
  /* Add and get primitive */
  Group group;
  group_init(&group);

  _Optional Primitive *p = group_add_primitive(&group);
  assert(p != NULL);
  Primitive *const primitive = &*p;
  assert(group_get_num_primitives(&group) == 1);
  assert(group_get_primitive(&group, 0) == primitive);
  assert(group_get_primitive(&group, -1) == NULL);
  assert(group_get_primitive(&group, 1) == NULL);

  group_free(&group);
}

static void test4(void)
{
  /* Insert primitive */
  Group group;
  group_init(&group);

  _Optional Primitive *a = group_add_primitive(&group);
  _Optional Primitive *b = group_add_primitive(&group);
  assert(a != NULL && b != NULL);

  _Optional Primitive *first = group_get_primitive(&group, 0);
  _Optional Primitive *second = group_get_primitive(&group, 1);
  assert(first != NULL && second != NULL);
  primitive_set_id(&*first, 10);
  primitive_set_id(&*second, 20);

  _Optional Primitive *middle = group_insert_primitive(&group, 1);
  assert(middle != NULL);
  primitive_set_id(&*middle, 15);

  assert(group_get_num_primitives(&group) == 3);
  for (int i = 0; i < 3; ++i)
  {
    _Optional Primitive *primitive = group_get_primitive(&group, i);
    assert(primitive != NULL);
    assert(primitive_get_id(&*primitive) == 10 + 5 * i);
  }
  assert(group_insert_primitive(&group, -1) == NULL);
  assert(group_insert_primitive(&group, 4) == NULL);

  group_free(&group);
}

static void test5(void)
{
  /* Delete primitive */
  Group group;
  group_init(&group);

  for (int i = 0; i < 3; ++i)
  {
    _Optional Primitive *p = group_add_primitive(&group);
    assert(p != NULL);
    primitive_set_id(&*p, i);
  }

  group_delete_primitive(&group, 1);
  assert(group_get_num_primitives(&group) == 2);
  _Optional Primitive *first = group_get_primitive(&group, 0);
  _Optional Primitive *second = group_get_primitive(&group, 1);
  assert(first != NULL && second != NULL);
  assert(primitive_get_id(&*first) == 0);
  assert(primitive_get_id(&*second) == 2);

  group_delete_primitive(&group, -1);
  group_delete_primitive(&group, 99);
  assert(group_get_num_primitives(&group) == 2);

  group_free(&group);
}

static void test6(void)
{
  /* Mark used vertices */
  Group group;
  VertexArray varray;
  Coord coords[3] = {1, 2, 3};

  group_init(&group);
  vertex_array_init(&varray);
  assert(vertex_array_add_vertex(&varray, &coords) == 0);

  _Optional Primitive *p = group_add_primitive(&group);
  assert(p != NULL);
  assert(primitive_add_side(&*p, 0) == 0);

  assert(!vertex_array_is_used(&varray, 0));
  group_set_used(&group, &varray);
  assert(vertex_array_is_used(&varray, 0));

  group_free(&group);
  vertex_array_free(&varray);
}

#ifdef FORTIFY
static void test7(void)
{
  /* Allocate fail recovery */
  Group group;
  group_init(&group);

  Fortify_SetNumAllocationsLimit(0);
  assert(group_alloc_primitives(&group, 1) == 0);
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(group_get_num_primitives(&group) == 0);
  assert(group_alloc_primitives(&group, 1) >= 1);
  group_free(&group);
}

static void test8(void)
{
  /* Add fail recovery */
  Group group;
  group_init(&group);

  Fortify_SetNumAllocationsLimit(0);
  assert(group_add_primitive(&group) == NULL);
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(group_get_num_primitives(&group) == 0);
  assert(group_add_primitive(&group) != NULL);
  assert(group_get_num_primitives(&group) == 1);
  group_free(&group);
}

static void test9(void)
{
  /* Insert fail recovery */
  Group group;
  group_init(&group);

  for (int i = 0; i < 8; ++i)
    assert(group_add_primitive(&group) != NULL);

  Fortify_SetNumAllocationsLimit(0);
  assert(group_insert_primitive(&group, 4) == NULL);
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(group_get_num_primitives(&group) == 8);
  assert(group_insert_primitive(&group, 4) != NULL);
  assert(group_get_num_primitives(&group) == 9);
  group_free(&group);
}
#endif

void Group_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Init, delete all and free", test1},
    {"Allocation", test2},
    {"Add and get primitive", test3},
    {"Insert primitive", test4},
    {"Delete primitive", test5},
    {"Mark used vertices", test6},
#ifdef FORTIFY
    {"Allocate fail recovery", test7},
    {"Add fail recovery", test8},
    {"Insert fail recovery", test9},
#endif
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
