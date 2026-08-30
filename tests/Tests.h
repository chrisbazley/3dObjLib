/*
 * 3dObjLib test: Macro and test suite definitions
 * Copyright (C) 2026 Christopher Bazley
 */
#ifndef Tests_h
#define Tests_h

#undef NDEBUG

#include <assert.h>

#ifdef USE_OPTIONAL
#include "Optional.h"
#endif

#ifdef FORTIFY
#include "fortify.h"
#else
#define Fortify_EnterScope()
#define Fortify_LeaveScope()
#define Fortify_OutputStatistics()
#endif

#define NOT_USED(x) ((void)(x))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

void Coord_tests(void);
void Vector_tests(void);
void Vertex_tests(void);
void Primitive_tests(void);
void Group_tests(void);
void ObjFile_tests(void);
void Clip_tests(void);

#endif /* Tests_h */
