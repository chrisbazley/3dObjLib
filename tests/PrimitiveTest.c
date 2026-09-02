/*
 * 3dObjLib test: Geometric primitive storage
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* 3dObjLib headers */
#include "Primitive.h"

/* Local headers */
#include "Tests.h"

static int add_vertex(VertexArray *varray, Coord x, Coord y, Coord z)
{
  Coord coords[3] = {x, y, z};
  return vertex_array_add_vertex(varray, &coords);
}

static void make_square(VertexArray *varray, Primitive *primitive,
                        Coord x0, Coord y0, Coord x1, Coord y1)
{
  primitive_init(primitive);
  assert(primitive_add_side(primitive, add_vertex(varray, x0, y0, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, x1, y0, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, x1, y1, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, x0, y1, 0)) >= 0);
}


static void make_triangle(VertexArray *varray, Primitive *primitive,
                          Coord ax, Coord ay, Coord bx, Coord by,
                          Coord cx, Coord cy)
{
  primitive_init(primitive);
  assert(primitive_add_side(primitive, add_vertex(varray, ax, ay, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, bx, by, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, cx, cy, 0)) >= 0);
}

static void make_pentagon(VertexArray *varray, Primitive *primitive)
{
  primitive_init(primitive);
  assert(primitive_add_side(primitive, add_vertex(varray, 0, 2, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 2, 0, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 5, 1, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 4, 4, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 1, 5, 0)) >= 0);
}

static void make_concave_pentagon(VertexArray *varray, Primitive *primitive)
{
  primitive_init(primitive);
  assert(primitive_add_side(primitive, add_vertex(varray, 0, 0, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 4, 0, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 2, 2, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 4, 4, 0)) >= 0);
  assert(primitive_add_side(primitive, add_vertex(varray, 0, 4, 0)) >= 0);
}

static void test1(void)
{
  /* Init and attributes */
  Primitive primitive;
  memset(&primitive, CHAR_MAX, sizeof(primitive));

  primitive_init(&primitive);
  assert(primitive_get_num_sides(&primitive) == 0);
  assert(primitive_get_colour(&primitive) == 0);
  assert(primitive_get_id(&primitive) == 0);

  primitive_set_colour(&primitive, 42);
  primitive_set_id(&primitive, 99);
  assert(primitive_get_colour(&primitive) == 42);
  assert(primitive_get_id(&primitive) == 99);

  primitive_set_id(&primitive, -1);
  assert(primitive_get_id(&primitive) == 99);
}

static void test2(void)
{
  /* Sides */
  Primitive primitive;
  primitive_init(&primitive);

  assert(primitive_get_side(&primitive, 0) == -1);
  assert(primitive_add_side(&primitive, -1) == -1);

  for (int i = 0; i < 15; ++i)
    assert(primitive_add_side(&primitive, 100 + i) == i);

  assert(primitive_get_num_sides(&primitive) == 15);
  assert(primitive_get_side(&primitive, 0) == 100);
  assert(primitive_get_side(&primitive, 14) == 114);
  assert(primitive_add_side(&primitive, 999) == -1);

  primitive_delete_all(&primitive);
  assert(primitive_get_num_sides(&primitive) == 0);
}

static void test3(void)
{
  /* Reverse sides */
  Primitive primitive;
  primitive_init(&primitive);

  for (int i = 0; i < 5; ++i)
    assert(primitive_add_side(&primitive, i) == i);

  primitive_reverse_sides(&primitive);
  for (int i = 0; i < 5; ++i)
    assert(primitive_get_side(&primitive, i) == 4 - i);

  primitive_reverse_sides(&primitive);
  for (int i = 0; i < 5; ++i)
    assert(primitive_get_side(&primitive, i) == i);
}

static void test4(void)
{
  /* Normal and plane */
  VertexArray varray;
  Primitive primitive;
  Coord norm[3];
  Plane plane;

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);

  assert(primitive_get_normal(&primitive, &varray, &norm));
  assert(coord_equal(norm[0], 0));
  assert(coord_equal(norm[1], 0));
  assert(coord_abs(norm[2]) > 0);
  assert(primitive_find_plane(&primitive, &varray, &plane));
  assert(plane.z == 2);

  vertex_array_free(&varray);
}

static void test5(void)
{
  /* Set normal */
  VertexArray varray;
  Primitive primitive;
  Coord norm[3], reverse[3];

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);

  assert(primitive_get_normal(&primitive, &varray, &norm));
  for (size_t i = 0; i < ARRAY_SIZE(norm); ++i)
    reverse[i] = -norm[i];

  assert(!primitive_set_normal(&primitive, &varray, &norm));
  assert(primitive_set_normal(&primitive, &varray, &reverse));

  {
    Coord got[3];
    assert(primitive_get_normal(&primitive, &varray, &got));
    assert(vector_equal(&got, &reverse));
  }

  vertex_array_free(&varray);
}

static void test6(void)
{
  /* Convexity */
  VertexArray varray;
  Primitive primitive;
  Plane plane = {0, 1, 2};

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);
  assert(primitive_is_convex(&primitive, &varray, plane));

  primitive_delete_all(&primitive);
  assert(primitive_add_side(&primitive, 0) == 0);
  assert(primitive_add_side(&primitive, 1) == 1);
  assert(!primitive_is_convex(&primitive, &varray, plane));

  vertex_array_free(&varray);
}

static void test7(void)
{
  /* Coplanarity */
  VertexArray varray;
  Primitive a, b, c;

  vertex_array_init(&varray);
  make_square(&varray, &a, 0, 0, 4, 4);
  make_square(&varray, &b, 1, 1, 3, 3);

  primitive_init(&c);
  assert(primitive_add_side(&c, add_vertex(&varray, 0, 0, 1)) >= 0);
  assert(primitive_add_side(&c, add_vertex(&varray, 4, 0, 1)) >= 0);
  assert(primitive_add_side(&c, add_vertex(&varray, 4, 4, 1)) >= 0);

  assert(primitive_coplanar(&a, &b, &varray));
  assert(!primitive_coplanar(&a, &c, &varray));

  vertex_array_free(&varray);
}

static void test8(void)
{
  /* Point containment */
  VertexArray varray;
  Primitive primitive;
  Plane plane = {0, 1, 2};
  Coord inside[3] = {2, 2, 0},
        edge[3] = {0, 2, 0},
        outside[3] = {5, 2, 0};

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);

  assert(primitive_contains_point(&primitive, &varray, &inside, plane));
  assert(primitive_contains_point(&primitive, &varray, &edge, plane));
  assert(!primitive_contains_point(&primitive, &varray, &outside, plane));

  vertex_array_free(&varray);
}

static void test9(void)
{
  /* Primitive containment */
  VertexArray varray;
  Primitive outer, inner;
  Plane plane = {0, 1, 2};

  vertex_array_init(&varray);
  make_square(&varray, &outer, 0, 0, 4, 4);
  make_square(&varray, &inner, 1, 1, 3, 3);

  assert(primitive_contains(&outer, &inner, &varray, plane));
  assert(!primitive_contains(&inner, &outer, &varray, plane));

  vertex_array_free(&varray);
}

static void test10(void)
{
  /* Primitive equality */
  Primitive a, b;
  primitive_init(&a);
  primitive_init(&b);

  assert(primitive_equal(&a, &b));

  assert(primitive_add_side(&a, 1) == 0);
  assert(primitive_add_side(&a, 2) == 1);
  assert(primitive_add_side(&a, 3) == 2);

  assert(primitive_add_side(&b, 2) == 0);
  assert(primitive_add_side(&b, 3) == 1);
  assert(primitive_add_side(&b, 1) == 2);
  assert(primitive_equal(&a, &b));

  primitive_reverse_sides(&b);
  assert(!primitive_equal(&a, &b));
}

static void test11(void)
{
  /* Intersection */
  VertexArray varray;
  Primitive square;
  Plane plane = {0, 1, 2};

  vertex_array_init(&varray);
  make_square(&varray, &square, 0, 0, 4, 4);

  const int a = add_vertex(&varray, -1, 2, 0),
            b = add_vertex(&varray, 5, 2, 0);
  assert(primitive_intersect(&square, a, b, &varray, plane));

  const int c = add_vertex(&varray, -1, 5, 0),
            d = add_vertex(&varray, 5, 5, 0);
  assert(!primitive_intersect(&square, c, d, &varray, plane));

  vertex_array_free(&varray);
}

static void test12(void)
{
  /* Split */
  VertexArray varray;
  Primitive square, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &square, 0, 0, 4, 4);
  const int a = add_vertex(&varray, 2, -1, 0),
            b = add_vertex(&varray, 2, 5, 0);

  assert(primitive_split(&square, a, b, &varray, plane, &other, &split));
  assert(split);
  assert(primitive_get_num_sides(&square) >= 3);
  assert(primitive_get_num_sides(&other) >= 3);

  vertex_array_free(&varray);
}

static void test13(void)
{
  /* Clip */
  VertexArray varray;
  Primitive back, front, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &back, 0, 0, 4, 4);
  make_square(&varray, &front, 2, 1, 5, 3);

  assert(primitive_clip(&back, &front, &varray, plane, &other, &split));
  assert(split);

  vertex_array_free(&varray);
}

static void test14(void)
{
  /* Mark used vertices */
  VertexArray varray;
  Primitive primitive;

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);

  primitive_set_used(&primitive, &varray);
  for (int side = 0; side < primitive_get_num_sides(&primitive); ++side)
    assert(vertex_array_is_used(&varray, primitive_get_side(&primitive, side)));

  vertex_array_free(&varray);
}

static void test15(void)
{
  /* Skew side */
  VertexArray varray;
  Primitive primitive;

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);
  assert(primitive_get_skew_side(&primitive, &varray) == -1);

  vertex_array_free(&varray);
}

static void test16(void)
{
  /* Print primitive */
  VertexArray varray;
  Primitive primitive;

  vertex_array_init(&varray);
  make_square(&varray, &primitive, 0, 0, 4, 4);
  primitive_print(&primitive, &varray);
  putchar('\n');

  vertex_array_free(&varray);
}


static void test17(void)
{
  /* Point and line have no plane */
  VertexArray varray;
  Primitive primitive;
  Plane plane;

  vertex_array_init(&varray);
  primitive_init(&primitive);

  assert(primitive_add_side(&primitive, add_vertex(&varray, 0, 0, 0)) == 0);
  assert(!primitive_find_plane(&primitive, &varray, &plane));

  assert(primitive_add_side(&primitive, add_vertex(&varray, 1, 0, 0)) == 1);
  assert(!primitive_find_plane(&primitive, &varray, &plane));

  vertex_array_free(&varray);
}

static void test18(void)
{
  /* Triangles and higher-order convex polygons */
  VertexArray varray;
  Primitive triangle, pentagon, concave;
  Plane plane = {0, 1, 2};

  vertex_array_init(&varray);
  make_triangle(&varray, &triangle, 0, 0, 4, 0, 0, 4);
  make_pentagon(&varray, &pentagon);
  make_concave_pentagon(&varray, &concave);

  assert(primitive_is_convex(&triangle, &varray, plane));
  assert(primitive_is_convex(&pentagon, &varray, plane));
  assert(!primitive_is_convex(&concave, &varray, plane));

  vertex_array_free(&varray);
}

static void test19(void)
{
  /* Triangle and pentagon containment */
  VertexArray varray;
  Primitive outer, triangle, pentagon;
  Plane plane = {0, 1, 2};

  vertex_array_init(&varray);
  make_square(&varray, &outer, -1, -1, 6, 6);
  make_triangle(&varray, &triangle, 0, 0, 4, 0, 0, 4);
  make_pentagon(&varray, &pentagon);

  assert(primitive_contains(&outer, &triangle, &varray, plane));
  assert(primitive_contains(&outer, &pentagon, &varray, plane));

  vertex_array_free(&varray);
}

static void test20(void)
{
  /* Point containment edge cases */
  VertexArray varray;
  Primitive triangle;
  Plane plane = {0, 1, 2};
  Coord vertex[3] = {0, 0, 0},
        horizontal[3] = {2, 0, 0},
        sloping[3] = {2, 2, 0},
        inside[3] = {1, 1, 0},
        outside_bbox[3] = {5, 5, 0};

  vertex_array_init(&varray);
  make_triangle(&varray, &triangle, 0, 0, 4, 0, 0, 4);

  assert(primitive_contains_point(&triangle, &varray, &vertex, plane));
  assert(primitive_contains_point(&triangle, &varray, &horizontal, plane));
  assert(primitive_contains_point(&triangle, &varray, &sloping, plane));
  assert(primitive_contains_point(&triangle, &varray, &inside, plane));
  assert(!primitive_contains_point(&triangle, &varray, &outside_bbox, plane));

  vertex_array_free(&varray);
}

static void test21(void)
{
  /* Intersection endpoint cases */
  VertexArray varray;
  Primitive square, line;
  Plane plane = {0, 1, 2};

  vertex_array_init(&varray);
  make_square(&varray, &square, 0, 0, 4, 4);

  primitive_init(&line);
  const int l0 = add_vertex(&varray, -1, 2, 0),
            l1 = add_vertex(&varray, 5, 2, 0);
  assert(primitive_add_side(&line, l0) == 0);
  assert(primitive_add_side(&line, l1) == 1);
  assert(!primitive_intersect(&line, l0, l1, &varray, plane));

  /* Sharing a polygon vertex is explicitly not an intersection. */
  const int shared = primitive_get_side(&square, 0),
            outside = add_vertex(&varray, -1, -1, 0);
  assert(!primitive_intersect(&square, shared, outside, &varray, plane));

  /*
   * An intersection coincident with an endpoint of the supplied edge is
   * likewise excluded, even if that endpoint is not one of the polygon's
   * vertex IDs.
   */
  const int coincident = add_vertex(&varray, 0, 2, 0),
            farther = add_vertex(&varray, -2, 2, 0);
  assert(!primitive_intersect(&square, coincident, farther, &varray, plane));

  /* Passing through a corner in the middle of an edge still counts. */
  const int cross_a = add_vertex(&varray, -1, -1, 0),
            cross_b = add_vertex(&varray, 1, 1, 0);
  assert(primitive_intersect(&square, cross_a, cross_b, &varray, plane));

  vertex_array_free(&varray);
}

static void test22(void)
{
  /* Split unsupported and no-split cases */
  VertexArray varray;
  Primitive line, square, other;
  Plane plane = {0, 1, 2};
  bool split = true;

  vertex_array_init(&varray);

  primitive_init(&line);
  assert(primitive_add_side(&line, add_vertex(&varray, 0, 0, 0)) == 0);
  assert(primitive_add_side(&line, add_vertex(&varray, 1, 0, 0)) == 1);
  const int a = add_vertex(&varray, 0, -1, 0),
            b = add_vertex(&varray, 0, 1, 0);
  assert(primitive_split(&line, a, b, &varray, plane, &other, &split));
  assert(!split);

  make_square(&varray, &square, 0, 0, 4, 4);
  const int c = add_vertex(&varray, 5, -1, 0),
            d = add_vertex(&varray, 5, 5, 0);
  split = true;
  assert(primitive_split(&square, c, d, &varray, plane, &other, &split));
  assert(!split);

  vertex_array_free(&varray);
}

static void test23(void)
{
  /* Split through existing vertices */
  VertexArray varray;
  Primitive square, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &square, 0, 0, 4, 4);

  const int a = primitive_get_side(&square, 0),
            b = primitive_get_side(&square, 2);
  const int before = vertex_array_get_num_vertices(&varray);

  assert(primitive_split(&square, a, b, &varray, plane, &other, &split));
  assert(split);
  assert(vertex_array_get_num_vertices(&varray) == before);
  assert(primitive_get_num_sides(&square) == 3);
  assert(primitive_get_num_sides(&other) == 3);

  vertex_array_free(&varray);
}

static void test24(void)
{
  /* Split preserves a cached normal */
  VertexArray varray;
  Primitive square, other;
  Plane plane = {0, 1, 2};
  Coord before[3], after[3];
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &square, 0, 0, 4, 4);
  assert(primitive_get_normal(&square, &varray, &before));

  const int a = add_vertex(&varray, 2, -1, 0),
            b = add_vertex(&varray, 2, 5, 0);
  assert(primitive_split(&square, a, b, &varray, plane, &other, &split));
  assert(split);
  assert(other.has_normal);
  assert(primitive_get_normal(&other, &varray, &after));
  assert(vector_equal(&before, &after));

  vertex_array_free(&varray);
}

static void test25(void)
{
  /* Clip with point or line */
  VertexArray varray;
  Primitive back, clipper, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &back, 0, 0, 4, 4);

  primitive_init(&clipper);
  assert(primitive_add_side(&clipper, add_vertex(&varray, 1, 1, 0)) == 0);
  assert(!primitive_clip(&back, &clipper, &varray, plane, &other, &split));

  assert(primitive_add_side(&clipper, add_vertex(&varray, 3, 1, 0)) == 1);
  assert(!primitive_clip(&back, &clipper, &varray, plane, &other, &split));

  vertex_array_free(&varray);
}

static void test26(void)
{
  /* Valid clip with no overlap */
  VertexArray varray;
  Primitive back, front, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &back, 0, 0, 2, 2);
  make_triangle(&varray, &front, 3, 3, 5, 3, 3, 5);

  assert(primitive_clip(&back, &front, &varray, plane, &other, &split));
  assert(!split);

  vertex_array_free(&varray);
}

static void test27(void)
{
  /* Overlapping bounding boxes without polygon overlap */
  VertexArray varray;
  Primitive back, front, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_triangle(&varray, &back, 0, 0, 4, 0, 0, 4);
  make_triangle(&varray, &front, 3, 3, 5, 3, 3, 5);

  assert(primitive_clip(&back, &front, &varray, plane, &other, &split));
  assert(!split);

  vertex_array_free(&varray);
}

static void test28(void)
{
  /* Triangle clipping */
  VertexArray varray;
  Primitive back, front, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_triangle(&varray, &back, 0, 0, 5, 0, 0, 5);
  make_triangle(&varray, &front, 2, -1, 6, 1, 2, 3);

  assert(primitive_clip(&back, &front, &varray, plane, &other, &split));
  assert(split);

  vertex_array_free(&varray);
}

static void test29(void)
{
  /* Higher-order polygon clipping */
  VertexArray varray;
  Primitive back, front, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_pentagon(&varray, &back);
  make_triangle(&varray, &front, 2, -1, 6, 1, 2, 3);

  assert(primitive_clip(&back, &front, &varray, plane, &other, &split));
  assert(split);

  vertex_array_free(&varray);
}

static void test30(void)
{
  /* Positive skew detection */
  VertexArray varray;
  Primitive primitive;

  vertex_array_init(&varray);
  primitive_init(&primitive);
  assert(primitive_add_side(&primitive, add_vertex(&varray, 0, 0, 0)) == 0);
  assert(primitive_add_side(&primitive, add_vertex(&varray, 4, 0, 0)) == 1);
  assert(primitive_add_side(&primitive, add_vertex(&varray, 4, 4, 0)) == 2);
  assert(primitive_add_side(&primitive, add_vertex(&varray, 0, 4, 1)) == 3);

  assert(primitive_get_skew_side(&primitive, &varray) == 3);

  vertex_array_free(&varray);
}

#ifdef FORTIFY
static void test31(void)
{
  /* Split fail recovery */
  VertexArray varray;
  Primitive square, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &square, 0, 0, 4, 4);
  const int a = add_vertex(&varray, 2, -1, 0),
            b = add_vertex(&varray, 2, 5, 0);

  /* Fill the initial allocation so adding an intersection must grow it. */
  while (vertex_array_get_num_vertices(&varray) < varray.nalloc)
    assert(add_vertex(&varray, 100 + vertex_array_get_num_vertices(&varray),
                      100, 0) >= 0);

  const int before = vertex_array_get_num_vertices(&varray);
  Primitive saved = square;

  Fortify_SetNumAllocationsLimit(0);
  assert(!primitive_split(&square, a, b, &varray, plane, &other, &split));
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  assert(vertex_array_get_num_vertices(&varray) == before);
  square = saved;
  split = false;
  assert(primitive_split(&square, a, b, &varray, plane, &other, &split));
  assert(split);

  vertex_array_free(&varray);
}

static void test32(void)
{
  /* Clip fail recovery */
  VertexArray varray;
  Primitive back, front, other;
  Plane plane = {0, 1, 2};
  bool split = false;

  vertex_array_init(&varray);
  make_square(&varray, &back, 0, 0, 4, 4);
  make_square(&varray, &front, 2, 1, 5, 3);

  while (vertex_array_get_num_vertices(&varray) < varray.nalloc)
    assert(add_vertex(&varray, 100 + vertex_array_get_num_vertices(&varray),
                      100, 0) >= 0);

  Primitive saved = back;

  Fortify_SetNumAllocationsLimit(0);
  assert(!primitive_clip(&back, &front, &varray, plane, &other, &split));
  Fortify_SetNumAllocationsLimit(ULONG_MAX);

  back = saved;
  split = false;
  assert(primitive_clip(&back, &front, &varray, plane, &other, &split));
  assert(split);

  vertex_array_free(&varray);
}
#endif

void Primitive_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Init and attributes", test1},
    {"Sides", test2},
    {"Reverse sides", test3},
    {"Normal and plane", test4},
    {"Set normal", test5},
    {"Convexity", test6},
    {"Coplanarity", test7},
    {"Point containment", test8},
    {"Primitive containment", test9},
    {"Primitive equality", test10},
    {"Intersection", test11},
    {"Split", test12},
    {"Clip", test13},
    {"Mark used vertices", test14},
    {"Skew side", test15},
    {"Print primitive", test16},
    {"Point and line have no plane", test17},
    {"Triangles and higher-order convex polygons", test18},
    {"Triangle and pentagon containment", test19},
    {"Point containment edge cases", test20},
    {"Intersection endpoint cases", test21},
    {"Split unsupported and no-split cases", test22},
    {"Split through existing vertices", test23},
    {"Split preserves a cached normal", test24},
    {"Clip with point or line", test25},
    {"Valid clip with no overlap", test26},
    {"Overlapping bounding boxes without polygon overlap", test27},
    {"Triangle clipping", test28},
    {"Higher-order polygon clipping", test29},
    {"Positive skew detection", test30},
#ifdef FORTIFY
    {"Split fail recovery", test31},
    {"Clip fail recovery", test32},
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
