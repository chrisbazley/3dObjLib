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

#define COORD_INF ((Coord)INFINITY)

typedef double Coord;

static inline Coord coord_abs(const Coord a)
{
  return fabs(a);
}

static inline Coord coord_sqrt(const Coord a)
{
  return sqrt(a);
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
