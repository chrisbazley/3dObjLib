/*
 * 3dObjLib test: main program
 * Copyright (C) 2026 Christopher Bazley
 */

/* ISO library headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local headers */
#include "Tests.h"

#ifdef FORTIFY
static int fortify_detected;
static Fortify_OutputFuncPtr fortify_previous_output;

static void fortify_check(void)
{
  Fortify_CheckAllMemory();
  assert(!fortify_detected);
}

static void fortify_output(const char *text)
{
  fortify_previous_output(text);
  if (strstr(text, "detected"))
    fortify_detected = 1;
}
#endif

int main(int argc, char *argv[])
{
  static const struct
  {
    const char *test_name;
    void (*test_func)(void);
  } test_groups[] = {
    {"Coord", Coord_tests},
    {"Vector", Vector_tests},
    {"Vertex", Vertex_tests},
    {"Primitive", Primitive_tests},
    {"Group", Group_tests},
    {"ObjFile", ObjFile_tests},
    {"Clip", Clip_tests},
  };

  NOT_USED(argc);
  NOT_USED(argv);

#ifdef FORTIFY
  fortify_previous_output = Fortify_SetOutputFunc(fortify_output);
  atexit(fortify_check);
#endif

  for (size_t count = 0; count < ARRAY_SIZE(test_groups); count++)
  {
    const size_t len = strlen(test_groups[count].test_name);
    puts(test_groups[count].test_name);
    for (size_t i = 0; i < len; i++)
      putchar('-');
    putchar('\n');

    Fortify_EnterScope();
    test_groups[count].test_func();
    Fortify_LeaveScope();

    putchar('\n');
  }

  Fortify_OutputStatistics();
  return EXIT_SUCCESS;
}
