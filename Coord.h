/*
 * 3dObjLib: Coordinate data type
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
  CJB: 05-Aug-18: Copied this source file from SF3KtoObj.
  CJB: 26-Aug-18: Added a new function, coord_less_than.
  CJB: 15-Oct-26: Use single-precision maths with SDCC.
 */

#ifndef COORD_H
#define COORD_H

#include <stdbool.h>
#include <math.h>

/* This value has been tuned to allow single-precision floating-point
   arithmetic to be substituted for double-precision. If it's too small
   then the polygon clipping code breaks (e.g. by creating zero-length
   edges after failing to recognise equal vertex coordinates). */
#define MAX_FLT_ERR (0.001)

#define PCOORD "g"

#ifdef __SDCC
/* SDCC provides only single-precision maths and no INFINITY macro. */
#define COORD_INF (1.0f / 0.0f)
typedef float Coord;
#else
#define COORD_INF ((Coord)INFINITY)
typedef double Coord;
#endif

static inline Coord coord_abs(const Coord a)
{
#ifdef __SDCC
  return fabsf(a);
#else
  return fabs(a);
#endif
}

static inline Coord coord_sqrt(const Coord a)
{
#ifdef __SDCC
  return sqrtf(a);
#else
  return sqrt(a);
#endif
}

static inline bool coord_is_finite(const Coord a)
{
#ifdef __SDCC
  return !isinf(a) && !isnan(a);
#else
  return isfinite(a);
#endif
}

static inline bool coord_equal(const Coord a, const Coord b)
{
  return coord_abs(a - b) < MAX_FLT_ERR;
}

static inline bool coord_less_than(const Coord a, const Coord b)
{
  return (b - a) >= MAX_FLT_ERR;
}

#endif /* COORD_H */
