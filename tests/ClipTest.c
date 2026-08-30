/*
 * 3dObjLib test: Polygon clipping
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <limits.h>
#include <stdio.h>

/* 3dObjLib headers */
#include "Clip.h"

/* Local headers */
#include "Tests.h"

static int add_vertex(VertexArray *varray, Coord x, Coord y)
{
  Coord coords[3] = {x, y, 0};
  return vertex_array_add_vertex(varray, &coords);
}

static void add_square(Group *group, VertexArray *varray,
                       Coord x0, Coord y0, Coord x1, Coord y1)
{
  _Optional Primitive *primitive = group_add_primitive(group);
  assert(primitive != NULL);
  Primitive *const checked = &*primitive;
  assert(primitive_add_side(checked, add_vertex(varray, x0, y0)) >= 0);
  assert(primitive_add_side(checked, add_vertex(varray, x1, y0)) >= 0);
  assert(primitive_add_side(checked, add_vertex(varray, x1, y1)) >= 0);
  assert(primitive_add_side(checked, add_vertex(varray, x0, y1)) >= 0);
}


static int add_vertex_z(VertexArray *varray, Coord x, Coord y, Coord z)
{
  Coord coords[3] = {x, y, z};
  return vertex_array_add_vertex(varray, &coords);
}

static Primitive *add_empty_primitive(Group *group)
{
  _Optional Primitive *primitive = group_add_primitive(group);
  assert(primitive != NULL);
  return &*primitive;
}

static void add_triangle(Group *group, VertexArray *varray,
                         Coord ax, Coord ay, Coord bx, Coord by,
                         Coord cx, Coord cy)
{
  Primitive *const primitive = add_empty_primitive(group);
  assert(primitive_add_side(primitive, add_vertex(varray, ax, ay)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, bx, by)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, cx, cy)) >= 0);
}

static void add_pentagon(Group *group, VertexArray *varray)
{
  Primitive *const primitive = add_empty_primitive(group);
  assert(primitive_add_side(primitive, add_vertex(varray, 0, 2)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 2, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 5, 1)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 4, 4)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 1, 5)) >= 0);
}

static void add_concave_pentagon(Group *group, VertexArray *varray)
{
  Primitive *const primitive = add_empty_primitive(group);
  assert(primitive_add_side(primitive, add_vertex(varray, 0, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 4, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 2, 2)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 4, 4)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 0, 4)) >= 0);
}

static void add_line(Group *group, VertexArray *varray,
                     Coord ax, Coord ay, Coord bx, Coord by)
{
  Primitive *const primitive = add_empty_primitive(group);
  assert(primitive_add_side(primitive, add_vertex(varray, ax, ay)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, bx, by)) >= 0);
}

static void add_point(Group *group, VertexArray *varray, Coord x, Coord y)
{
  Primitive *const primitive = add_empty_primitive(group);
  assert(primitive_add_side(primitive, add_vertex(varray, x, y)) >= 0);
}

static void add_square_z(Group *group, VertexArray *varray,
                         Coord x0, Coord y0, Coord x1, Coord y1, Coord z)
{
  Primitive *const primitive = add_empty_primitive(group);
  assert(primitive_add_side(primitive, add_vertex_z(varray, x0, y0, z)) >= 0);
  assert(primitive_add_side(primitive, add_vertex_z(varray, x1, y0, z)) >= 0);
  assert(primitive_add_side(primitive, add_vertex_z(varray, x1, y1, z)) >= 0);
  assert(primitive_add_side(primitive, add_vertex_z(varray, x0, y1, z)) >= 0);
}

static void copy_primitive(Group *group, const Primitive *source)
{
  Primitive *const copy = add_empty_primitive(group);
  const int nsides = primitive_get_num_sides(source);
  for (int side = 0; side < nsides; ++side)
    assert(primitive_add_side(copy, primitive_get_side(source, side)) >= 0);
}

static void test1(void)
{
  /* No groups */
  VertexArray varray;
  Group group;
  const int order[] = {0};

  vertex_array_init(&varray);
  group_init(&group);

  assert(clip_polygons(&varray, &group, order, 0, false));

  group_free(&group);
  vertex_array_free(&varray);
}

static void test2(void)
{
  /* Non-overlapping polygons unchanged */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 2, 2);
  add_square(&groups[1], &varray, 3, 3, 5, 5);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 1);
  assert(group_get_num_primitives(&groups[1]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test3(void)
{
  /* Covered polygon deleted */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 1, 1, 3, 3);
  add_square(&groups[1], &varray, 0, 0, 4, 4);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 0);
  assert(group_get_num_primitives(&groups[1]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test4(void)
{
  /* Partial overlap is split */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 4, 4);
  add_square(&groups[1], &varray, 2, 1, 5, 3);
  const int old_nvertices = vertex_array_get_num_vertices(&varray);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(vertex_array_get_num_vertices(&varray) > old_nvertices);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}


static void test5(void)
{
  /* Point and line back primitives are ignored */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_point(&groups[0], &varray, 1, 1);
  add_line(&groups[0], &varray, 0, 2, 4, 2);
  add_square(&groups[1], &varray, 0, 0, 4, 4);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 2);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test6(void)
{
  /* Point and line front primitives are ignored */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 4, 4);
  add_point(&groups[1], &varray, 1, 1);
  add_line(&groups[1], &varray, 0, 2, 4, 2);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 1);
  assert(group_get_num_primitives(&groups[1]) == 2);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test7(void)
{
  /* Equal polygon is deleted */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 4, 4);
  _Optional Primitive *back = group_get_primitive(&groups[0], 0);
  assert(back != NULL);
  copy_primitive(&groups[1], &*back);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 0);
  assert(group_get_num_primitives(&groups[1]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test8(void)
{
  /* Non-coplanar polygons are ignored */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square_z(&groups[0], &varray, 0, 0, 4, 4, 0);
  add_square_z(&groups[1], &varray, 0, 0, 4, 4, 1);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 1);
  assert(group_get_num_primitives(&groups[1]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test9(void)
{
  /* Concave back polygon is ignored */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_concave_pentagon(&groups[0], &varray);
  add_square(&groups[1], &varray, 1, 1, 3, 3);

  assert(clip_polygons(&varray, groups, order, 2, true));
  assert(group_get_num_primitives(&groups[0]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test10(void)
{
  /* Concave front polygon is ignored */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 1, 1, 3, 3);
  add_concave_pentagon(&groups[1], &varray);

  assert(clip_polygons(&varray, groups, order, 2, true));
  assert(group_get_num_primitives(&groups[0]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test11(void)
{
  /* Triangle against triangle */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_triangle(&groups[0], &varray, 0, 0, 5, 0, 0, 5);
  add_triangle(&groups[1], &varray, 2, -1, 6, 1, 2, 3);
  const int old_nvertices = vertex_array_get_num_vertices(&varray);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(vertex_array_get_num_vertices(&varray) > old_nvertices);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test12(void)
{
  /* Triangle against quadrilateral */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_triangle(&groups[0], &varray, 0, 0, 5, 0, 0, 5);
  add_square(&groups[1], &varray, 2, -1, 6, 2);
  const int old_nvertices = vertex_array_get_num_vertices(&varray);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(vertex_array_get_num_vertices(&varray) > old_nvertices);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test13(void)
{
  /* Quadrilateral against triangle */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 4, 4);
  add_triangle(&groups[1], &varray, 2, -1, 6, 1, 2, 3);
  const int old_nvertices = vertex_array_get_num_vertices(&varray);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(vertex_array_get_num_vertices(&varray) > old_nvertices);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test14(void)
{
  /* Higher-order polygon against triangle */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_pentagon(&groups[0], &varray);
  add_triangle(&groups[1], &varray, 2, -1, 6, 1, 2, 3);
  const int old_nvertices = vertex_array_get_num_vertices(&varray);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(vertex_array_get_num_vertices(&varray) > old_nvertices);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test15(void)
{
  /* Touching at a vertex does not overlap */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 2, 2);
  add_square(&groups[1], &varray, 2, 2, 4, 4);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test16(void)
{
  /* Sharing an edge does not overlap */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 2, 2);
  add_square(&groups[1], &varray, 2, 0, 4, 2);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test17(void)
{
  /* Same-group clipping copes with insertion moving the front polygon */
  VertexArray varray;
  Group group;
  const int order[] = {0};

  vertex_array_init(&varray);
  group_init(&group);

  add_square(&group, &varray, 0, 0, 4, 4);
  add_triangle(&group, &varray, 2, -1, 6, 1, 2, 3);
  const int old_nvertices = vertex_array_get_num_vertices(&varray);

  assert(clip_polygons(&varray, &group, order, 1, false));
  assert(vertex_array_get_num_vertices(&varray) > old_nvertices);

  group_free(&group);
  vertex_array_free(&varray);
}

static void test18(void)
{
  /* Repeated group number in plot order is not clipped against itself */
  VertexArray varray;
  Group group;
  const int order[] = {0, 0};

  vertex_array_init(&varray);
  group_init(&group);
  add_square(&group, &varray, 0, 0, 4, 4);

  assert(clip_polygons(&varray, &group, order, 2, false));
  assert(group_get_num_primitives(&group) == 1);

  group_free(&group);
  vertex_array_free(&varray);
}

static void test19(void)
{
  /* Overlapping bounding boxes without polygon overlap */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_triangle(&groups[0], &varray, 0, 0, 4, 0, 0, 4);
  add_triangle(&groups[1], &varray, 3, 3, 5, 3, 3, 5);

  assert(clip_polygons(&varray, groups, order, 2, false));
  assert(group_get_num_primitives(&groups[0]) == 1);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

#ifdef FORTIFY
static void test20(void)
{
  /* Intersection-vertex allocation fail recovery */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  add_square(&groups[0], &varray, 0, 0, 4, 4);
  add_square(&groups[1], &varray, 2, 1, 5, 3);
  assert(vertex_array_get_num_vertices(&varray) == varray.nalloc);

  Fortify_SetNumAllocationsLimit(0);
  assert(!clip_polygons(&varray, groups, order, 2, false));
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}

static void test21(void)
{
  /* Split-primitive allocation fail recovery */
  VertexArray varray;
  Group groups[2];
  const int order[] = {0, 1};

  vertex_array_init(&varray);
  group_init(&groups[0]);
  group_init(&groups[1]);

  /* Prevent intersection-vertex allocation from being the first failure. */
  assert(vertex_array_alloc_vertices(&varray, 64) >= 64);

  add_square(&groups[0], &varray, 0, 0, 4, 4);
  for (int i = 1; i < groups[0].nalloc; ++i)
  {
    _Optional Primitive *primitive = group_add_primitive(&groups[0]);
    assert(primitive != NULL);
  }
  add_square(&groups[1], &varray, 2, 1, 5, 3);

  /*
   * Two new intersection vertices are required before the completed split is
   * inserted into the full group. Permit those allocations (none are needed
   * because of the preallocation above), then fail the group realloc.
   */
  Fortify_SetNumAllocationsLimit(0);
  assert(!clip_polygons(&varray, groups, order, 2, false));
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  group_free(&groups[0]);
  group_free(&groups[1]);
  vertex_array_free(&varray);
}
#endif

void Clip_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"No groups", test1},
    {"Non-overlapping polygons unchanged", test2},
    {"Covered polygon deleted", test3},
    {"Partial overlap is split", test4},
    {"Point and line back primitives are ignored", test5},
    {"Point and line front primitives are ignored", test6},
    {"Equal polygon is deleted", test7},
    {"Non-coplanar polygons are ignored", test8},
    {"Concave back polygon is ignored", test9},
    {"Concave front polygon is ignored", test10},
    {"Triangle against triangle", test11},
    {"Triangle against quadrilateral", test12},
    {"Quadrilateral against triangle", test13},
    {"Higher-order polygon against triangle", test14},
    {"Touching at a vertex does not overlap", test15},
    {"Sharing an edge does not overlap", test16},
    {"Same-group clipping", test17},
    {"Repeated group number in plot order", test18},
    {"Overlapping bounding boxes without polygon overlap", test19},
#ifdef FORTIFY
    {"Intersection-vertex allocation fail recovery", test20},
    {"Split-primitive allocation fail recovery", test21},
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
