/*
 * 3dObjLib test: Vertex storage
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* 3dObjLib headers */
#include "Vertex.h"

/* Local headers */
#include "Tests.h"

static int add(VertexArray *varray, Coord x, Coord y, Coord z)
{
  Coord coords[3] = {x, y, z};
  return vertex_array_add_vertex(varray, &coords);
}

static void test1(void)
{
  /* Init, clear and free */
  VertexArray varray;
  memset(&varray, CHAR_MAX, sizeof(varray));

  vertex_array_init(&varray);
  assert(vertex_array_get_num_vertices(&varray) == 0);
  assert(vertex_array_get_vertex(&varray, 0) == NULL);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(vertex_array_get_num_vertices(&varray) == 1);

  vertex_array_clear(&varray);
  assert(vertex_array_get_num_vertices(&varray) == 0);
  assert(vertex_array_get_vertex(&varray, 0) == NULL);

  vertex_array_free(&varray);
}

static void test2(void)
{
  /* Allocation */
  VertexArray varray;
  vertex_array_init(&varray);

  assert(vertex_array_alloc_vertices(&varray, -1) == 0);
  assert(vertex_array_alloc_vertices(&varray, 1) >= 1);
  assert(vertex_array_alloc_vertices(&varray, 100) >= 100);

  vertex_array_free(&varray);
}

static void test3(void)
{
  /* Add and query */
  VertexArray varray;
  vertex_array_init(&varray);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(add(&varray, 4, 5, 6) == 1);
  assert(vertex_array_get_num_vertices(&varray) == 2);

  _Optional Vertex *v = vertex_array_get_vertex(&varray, 1);
  assert(v != NULL);
  Vertex *const vertex = &*v;
  assert(coord_equal(vertex->coords[0], 4));
  assert(coord_equal(vertex->coords[1], 5));
  assert(coord_equal(vertex->coords[2], 6));

  _Optional Coord (*coords)[3] = vertex_array_get_coords(&varray, 0);
  assert(coords != NULL);
  Coord (*const checked_coords)[3] = &*coords;
  assert(coord_equal((*checked_coords)[0], 1));
  assert(vertex_array_get_vertex(&varray, -1) == NULL);
  assert(vertex_array_get_vertex(&varray, 2) == NULL);
  assert(vertex_array_get_coords(&varray, 2) == NULL);

  vertex_array_free(&varray);
}

static void test4(void)
{
  /* Find vertex */
  VertexArray varray;
  Coord wanted[3] = {4, 5, 6}, absent[3] = {7, 8, 9};
  vertex_array_init(&varray);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(add(&varray, 4, 5, 6) == 1);
  assert(vertex_array_find_vertex(&varray, &wanted) == 1);
  assert(vertex_array_find_vertex(&varray, &absent) == -1);

  vertex_array_free(&varray);
}

static void test5(void)
{
  /* Used flags */
  VertexArray varray;
  vertex_array_init(&varray);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(add(&varray, 4, 5, 6) == 1);
  assert(!vertex_array_is_used(&varray, 0));
  assert(!vertex_array_is_used(&varray, 1));

  vertex_array_set_used(&varray, 1);
  assert(!vertex_array_is_used(&varray, 0));
  assert(vertex_array_is_used(&varray, 1));

  vertex_array_set_all_used(&varray);
  assert(vertex_array_is_used(&varray, 0));
  assert(vertex_array_is_used(&varray, 1));
  assert(!vertex_array_is_used(&varray, 99));

  vertex_array_free(&varray);
}

static void test6(void)
{
  /* Duplicate detection */
  VertexArray varray;
  vertex_array_init(&varray);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(add(&varray, 9, 9, 9) == 1);
  assert(add(&varray, 1, 2, 3) == 2);

  vertex_array_set_used(&varray, 2);
  assert(vertex_array_find_duplicates(&varray, false) == 1);
  assert(vertex_array_get_id(&varray, 2) == vertex_array_get_id(&varray, 0));
  assert(vertex_array_is_used(&varray, 0));
  assert(!vertex_array_is_used(&varray, 2));

  vertex_array_free(&varray);
}

static void test7(void)
{
  /* Renumber */
  VertexArray varray;
  vertex_array_init(&varray);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(add(&varray, 4, 5, 6) == 1);
  assert(add(&varray, 7, 8, 9) == 2);

  vertex_array_set_used(&varray, 0);
  vertex_array_set_used(&varray, 2);
  assert(vertex_array_renumber(&varray, false) == 2);
  assert(vertex_array_get_id(&varray, 0) == 0);
  assert(vertex_array_get_id(&varray, 2) == 1);

  vertex_array_free(&varray);
}

static void test8(void)
{
  /* Finite edge intersection */
  const Plane p = {0, 1, 2};
  VertexArray varray;
  Coord intersect[3];
  vertex_array_init(&varray);

  assert(add(&varray, 0, 0, 0) == 0);
  assert(add(&varray, 2, 2, 0) == 1);
  assert(add(&varray, 0, 2, 0) == 2);
  assert(add(&varray, 2, 0, 0) == 3);

  assert(vertex_array_edges_intersect(&varray, 0, 1, 2, 3, p, &intersect));
  assert(coord_equal(intersect[0], 1));
  assert(coord_equal(intersect[1], 1));

  vertex_array_free(&varray);
}

static void test9(void)
{
  /* Edge against infinite line */
  const Plane p = {0, 1, 2};
  VertexArray varray;
  Coord intersect[3];
  vertex_array_init(&varray);

  assert(add(&varray, 0, 0, 0) == 0);
  assert(add(&varray, 2, 2, 0) == 1);
  assert(add(&varray, 0, 2, 0) == 2);
  assert(add(&varray, 2, 0, 0) == 3);

  assert(vertex_array_edge_intersects_line(&varray, 0, 1, 2, 3, p,
                                           &intersect));
  assert(coord_equal(intersect[0], 1));
  assert(coord_equal(intersect[1], 1));

  vertex_array_free(&varray);
}

static void test10(void)
{
  /* Print vertex */
  VertexArray varray;
  vertex_array_init(&varray);
  assert(add(&varray, 1, 2, 3) == 0);
  vertex_array_print_vertex(&varray, 0);
  putchar('\n');
  vertex_array_free(&varray);
}

#ifdef FORTIFY
static void test11(void)
{
  /* Allocate fail recovery */
  VertexArray varray;
  vertex_array_init(&varray);

  Fortify_SetNumAllocationsLimit(0);
  assert(vertex_array_alloc_vertices(&varray, 1) == 0);
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(vertex_array_get_num_vertices(&varray) == 0);
  assert(vertex_array_alloc_vertices(&varray, 1) >= 1);
  vertex_array_free(&varray);
}

static void test12(void)
{
  /* Add fail recovery */
  VertexArray varray;
  vertex_array_init(&varray);

  Fortify_SetNumAllocationsLimit(0);
  assert(add(&varray, 1, 2, 3) == -1);
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(vertex_array_get_num_vertices(&varray) == 0);
  assert(add(&varray, 1, 2, 3) == 0);
  vertex_array_free(&varray);
}

static void test13(void)
{
  /* Duplicate-search fail recovery */
  VertexArray varray;
  vertex_array_init(&varray);

  assert(add(&varray, 1, 2, 3) == 0);
  assert(add(&varray, 1, 2, 3) == 1);

  Fortify_SetNumAllocationsLimit(0);
  assert(vertex_array_find_duplicates(&varray, false) == -1);
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(vertex_array_find_duplicates(&varray, false) == 1);
  vertex_array_free(&varray);
}
#endif

void Vertex_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Init, clear and free", test1},
    {"Allocation", test2},
    {"Add and query", test3},
    {"Find vertex", test4},
    {"Used flags", test5},
    {"Duplicate detection", test6},
    {"Renumber", test7},
    {"Finite edge intersection", test8},
    {"Edge against infinite line", test9},
    {"Print vertex", test10},
#ifdef FORTIFY
    {"Allocate fail recovery", test11},
    {"Add fail recovery", test12},
    {"Duplicate-search fail recovery", test13},
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
