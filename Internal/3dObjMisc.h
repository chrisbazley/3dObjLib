/*
 * 3dObjLib: Miscellaneous macro definitions
 * Copyright (C) 2018 Christopher Bazley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* History:
  CJB: 06-Apr-25: Dogfooding the _Optional qualifier.
 */

#ifndef M3dObjMisc_h
#define M3dObjMisc_h

/* Fortified memory allocation shell */
#ifdef FORTIFY
#include "Fortify.h"
#endif

#ifdef USE_OPTIONAL
#include <stdlib.h>

#undef NULL
#define NULL ((_Optional void *)0)

static inline void optional_free(_Optional void *x)
{
    free((void *)x);
}
#undef free
#define free(x) optional_free(x)

static inline _Optional void *optional_malloc(size_t n)
{
    return malloc(n);
}
#undef malloc
#define malloc(n) optional_malloc(n)

static inline _Optional void *optional_calloc(size_t sz, size_t n)
{
    return calloc(sz, n);
}
#undef calloc
#define calloc(n) optional_calloc(sz, n)

static inline _Optional void *optional_realloc(_Optional void *p, size_t n)
{
    return realloc((void *)p, n);
}
#undef realloc
#define realloc(p, n) optional_realloc(p, n)

#else
#define _Optional
#endif

#ifdef USE_CBDEBUG

#include "Debug.h"

#else /* USE_CBDEBUG */

#include <stdio.h>
#include <assert.h>

#ifdef DEBUG_OUTPUT
#define DEBUGF if (1) printf
#else
#define DEBUGF if (0) printf
#endif /* DEBUG_OUTPUT */

#endif /* USE_CBDEBUG */

#define NOT_USED(x) ((void)(x))

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define LOWEST(a, b) ((a) < (b) ? (a) : (b))

#define HIGHEST(a, b) ((a) > (b) ? (a) : (b))

#endif /* M3dObjMisc_h */
