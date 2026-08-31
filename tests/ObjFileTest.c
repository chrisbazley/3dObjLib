/*
 * 3dObjLib test: OBJ file generation
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <stdio.h>
#include <string.h>

/* 3dObjLib headers */
#include "ObjFile.h"

/* Local headers */
#include "Tests.h"

static int callback_context;

static int add_vertex(VertexArray *varray, Coord x, Coord y, Coord z)
{
  Coord coords[3] = {x, y, z};
  return vertex_array_add_vertex(varray, &coords);
}


static FILE *test_tmpfile(void)
{
  _Optional FILE *const file = tmpfile();
  if (file == NULL)
    perror("Failed to open temporary file");
  assert(file != NULL);
  return &*file;
}

static void read_file(FILE *file, char *buffer, size_t size)
{
  assert(file != NULL);
  assert(size > 0);
  assert(fflush(file) == 0);
  assert(fseek(file, 0, SEEK_SET) == 0);
  const size_t n = fread(buffer, 1, size - 1, file);
  buffer[n] = '\0';
}

static int get_colour(const Primitive *primitive, void *arg)
{
  int *calls = arg;
  ++*calls;
  return primitive_get_colour(primitive) + 1;
}

static int get_material(char *buffer, size_t buffer_size, int colour, void *arg)
{
  int *calls = arg;
  ++*calls;
  return snprintf(buffer, buffer_size, "material_%d", colour);
}

static void make_triangle(VertexArray *varray, Group *group)
{
  _Optional Primitive *primitive = group_add_primitive(group);
  assert(primitive != NULL);
  Primitive *const checked = &*primitive;
  assert(primitive_add_side(checked, add_vertex(varray, 1, 2, 3)) == 0);
  assert(primitive_add_side(checked, add_vertex(varray, 4, 5, 6)) == 1);
  assert(primitive_add_side(checked, add_vertex(varray, 7, 8, 9)) == 2);
  primitive_set_colour(checked, 5);
  vertex_array_set_all_used(varray);
}

static void test1(void)
{
  /* Output vertices */
  VertexArray varray;
  char text[1024];

  vertex_array_init(&varray);
  assert(add_vertex(&varray, 1, 2, 3) == 0);
  assert(add_vertex(&varray, -MAX_FLT_ERR / 2, 5, 6) == 1);
  vertex_array_set_all_used(&varray);

  FILE *const checked_file = test_tmpfile();
  assert(output_vertices(checked_file, 2, &varray, 1));
  read_file(checked_file, text, sizeof(text));

  _Optional const char *next = strstr(text, "# 2 vertices");
  assert(next != NULL);
  next = strstr(&*next, "v 1.000000 2.000000 3.000000");
  assert(next != NULL);
  next = strstr(&*next, "# Following vertices rotate");
  assert(next != NULL);
  next = strstr(&*next, "v 0.000000 5.000000 6.000000");
  assert(next != NULL);

  assert(!fclose(checked_file));
  vertex_array_free(&varray);
}

static void test2(void)
{
  /* Omit unused vertices */
  VertexArray varray;
  char text[1024];

  vertex_array_init(&varray);
  assert(add_vertex(&varray, 1, 2, 3) == 0);
  assert(add_vertex(&varray, 4, 5, 6) == 1);
  vertex_array_set_used(&varray, 1);

  FILE *const checked_file = test_tmpfile();
  assert(output_vertices(checked_file, 1, &varray, -1));
  read_file(checked_file, text, sizeof(text));

  assert(strstr(text, "1.000000 2.000000 3.000000") == NULL);
  assert(strstr(text, "4.000000 5.000000 6.000000") != NULL);

  assert(!fclose(checked_file));
  vertex_array_free(&varray);
}

static void test3(void)
{
  /* Positive primitive indices */
  VertexArray varray;
  Group group;
  char text[1024];

  vertex_array_init(&varray);
  group_init(&group);
  make_triangle(&varray, &group);

  FILE *const checked_file = test_tmpfile();
  assert(output_primitives(checked_file, "object", 10, 3, &varray, &group, 1,
                           0, 0, &callback_context,
                           VertexStyle_Positive, MeshStyle_NoChange));
  read_file(checked_file, text, sizeof(text));

  _Optional const char *next = strstr(text, "g object object_0");
  assert(next != NULL);
  next = strstr(&*next, "usemtl colour_5");
  assert(next != NULL);
  next = strstr(&*next, "f 11 12 13");
  assert(next != NULL);

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}

static void test4(void)
{
  /* Negative primitive indices */
  VertexArray varray;
  Group group;
  char text[1024];

  vertex_array_init(&varray);
  group_init(&group);
  make_triangle(&varray, &group);

  FILE *const checked_file = test_tmpfile();
  assert(output_primitives(checked_file, "object", 0, 3, &varray, &group, 1,
                           0, 0, &callback_context,
                           VertexStyle_Negative, MeshStyle_NoChange));
  read_file(checked_file, text, sizeof(text));

  _Optional const char *next = strstr(text, "g object object_0");
  assert(next != NULL);
  next = strstr(&*next, "usemtl colour_5");
  assert(next != NULL);
  next = strstr(&*next, "f -3 -2 -1");
  assert(next != NULL);

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}

static void test5(void)
{
  /* Colour and material callbacks */
  VertexArray varray;
  Group group;
  char text[1024];
  int calls = 0;

  vertex_array_init(&varray);
  group_init(&group);
  make_triangle(&varray, &group);

  FILE *const checked_file = test_tmpfile();
  assert(output_primitives(checked_file, "object", 0, 3, &varray, &group, 1,
                           get_colour, get_material, &calls,
                           VertexStyle_Positive, MeshStyle_NoChange));
  read_file(checked_file, text, sizeof(text));

  assert(calls == 2);
  _Optional const char *next = strstr(text, "g object object_0");
  assert(next != NULL);
  next = strstr(&*next, "usemtl material_6");
  assert(next != NULL);
  next = strstr(&*next, "f 1 2 3");
  assert(next != NULL);

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}

static void make_quad(VertexArray *varray, Group *group)
{
  _Optional Primitive *primitive = group_add_primitive(group);
  assert(primitive != NULL);
  Primitive *const checked = &*primitive;
  assert(primitive_add_side(checked, add_vertex(varray, 0, 0, 0)) == 0);
  assert(primitive_add_side(checked, add_vertex(varray, 4, 0, 0)) == 1);
  assert(primitive_add_side(checked, add_vertex(varray, 4, 4, 0)) == 2);
  assert(primitive_add_side(checked, add_vertex(varray, 0, 4, 0)) == 3);
  vertex_array_set_all_used(varray);
}

static void test6(void)
{
  /* Triangle fan */
  VertexArray varray;
  Group group;
  char text[1024];

  vertex_array_init(&varray);
  group_init(&group);
  make_quad(&varray, &group);

  FILE *const checked_file = test_tmpfile();
  assert(output_primitives(checked_file, "quad", 0, 4, &varray, &group, 1,
                           0, 0, &callback_context,
                           VertexStyle_Positive, MeshStyle_TriangleFan));
  read_file(checked_file, text, sizeof(text));

  _Optional const char *next = strstr(text, "g quad quad_0");
  assert(next != NULL);
  next = strstr(&*next, "usemtl colour_0");
  assert(next != NULL);
  next = strstr(&*next, "f 1 2 3");
  assert(next != NULL);
  next = strstr(&*next, "f 1 3 4");
  assert(next != NULL);

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}

static void test7(void)
{
  /* Triangle strip */
  VertexArray varray;
  Group group;
  char text[1024];

  vertex_array_init(&varray);
  group_init(&group);
  make_quad(&varray, &group);

  FILE *const checked_file = test_tmpfile();
  assert(output_primitives(checked_file, "quad", 0, 4, &varray, &group, 1,
                           0, 0, &callback_context,
                           VertexStyle_Positive, MeshStyle_TriangleStrip));
  read_file(checked_file, text, sizeof(text));

  _Optional const char *next = strstr(text, "g quad quad_0");
  assert(next != NULL);
  next = strstr(&*next, "usemtl colour_0");
  assert(next != NULL);
  next = strstr(&*next, "f 1 2 3");
  assert(next != NULL);
  next = strstr(&*next, "f 4 1 3");
  assert(next != NULL);

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}

static void test8(void)
{
  /* Empty object */
  VertexArray varray;
  Group group;

  vertex_array_init(&varray);
  group_init(&group);

  FILE *const checked_file = test_tmpfile();
  assert(output_vertices(checked_file, 0, &varray, -1));
  assert(output_primitives(checked_file, "empty", 0, 0, &varray, &group, 1,
                           0, 0, &callback_context,
                           VertexStyle_Positive, MeshStyle_NoChange));

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}


static void test9(void)
{
  /* Point and line primitives */
  VertexArray varray;
  Group group;
  char text[1024];

  vertex_array_init(&varray);
  group_init(&group);

  const int point_vertex = add_vertex(&varray, 1, 2, 3);
  assert(point_vertex == 0);
  _Optional Primitive *point = group_add_primitive(&group);
  assert(point != NULL);
  assert(primitive_add_side(&*point, point_vertex) == 0);

  const int line_vertex0 = add_vertex(&varray, 4, 5, 6);
  const int line_vertex1 = add_vertex(&varray, 7, 8, 9);
  assert(line_vertex0 == 1);
  assert(line_vertex1 == 2);
  _Optional Primitive *line = group_add_primitive(&group);
  assert(line != NULL);
  assert(primitive_add_side(&*line, line_vertex0) == 0);
  assert(primitive_add_side(&*line, line_vertex1) == 1);

  vertex_array_set_all_used(&varray);

  FILE *const checked_file = test_tmpfile();
  assert(output_primitives(checked_file, "object", 0, 3, &varray, &group, 1,
                           0, 0, &callback_context,
                           VertexStyle_Positive, MeshStyle_NoChange));
  read_file(checked_file, text, sizeof(text));

  _Optional const char *next = strstr(text, "g object object_0");
  assert(next != NULL);
  next = strstr(&*next, "usemtl colour_0");
  assert(next != NULL);
  next = strstr(&*next, "p 1\n");
  assert(next != NULL);
  next = strstr(&*next, "l 2 3\n");
  assert(next != NULL);

  assert(!fclose(checked_file));
  group_free(&group);
  vertex_array_free(&varray);
}

void ObjFile_tests(void)
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } unit_tests[] = {
    {"Output vertices", test1},
    {"Omit unused vertices", test2},
    {"Positive primitive indices", test3},
    {"Negative primitive indices", test4},
    {"Colour and material callbacks", test5},
    {"Triangle fan", test6},
    {"Triangle strip", test7},
    {"Empty object", test8},
    {"Point and line primitives", test9},
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
