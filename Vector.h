/*
 * 3dObjLib: Vector mathematics
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
  CJB: 26-Aug-18: Added a vector_xy_greater_or_equal function.
  CJB: 17-Nov-18: vector_x/y/z are no longer inline functions.
                  Plane indices are now bytes instead of size_t values.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdbool.h>
#include "Coord.h"

typedef struct {
  unsigned char x;
  unsigned char y;
  unsigned char z;
} Plane;

Coord *vector_x(Coord (*a)[3], Plane p);
Coord *vector_y(Coord (*a)[3], Plane p);
Coord *vector_z(Coord (*a)[3], Plane p);

void vector_mul(Coord (*vector)[3], Coord factor, Coord (*product)[3]);
void vector_add(Coord (*a)[3], Coord (*b)[3], Coord (*sum)[3]);
void vector_sub(Coord (*min)[3], Coord (*sub)[3], Coord (*diff)[3]);
void vector_cross(Coord (*a)[3], Coord (*b)[3], Coord (*prod)[3]);
Coord vector_mag(Coord (*a)[3]);
bool vector_norm(Coord (*a)[3], Coord (*unit)[3]);
Coord vector_dot(Coord (*a)[3], Coord (*b)[3]);
bool vector_equal(Coord (*a)[3], Coord (*b)[3]);

/* The following function is only likely to be useful for detecting overlap
   of two rectangles. In particular, !xy_less_than() is not equivalent to
   xy_greater_or_equal(), i.e. ax >= bx && ay >= by, as shown below:
  y
  |********     # : values of a for which ax < bx && ay < by
  |****b***     * : values of a for which !(ax < bx && ay < by)
  |####****         i.e. ax >= bx || ay >= by
 -|-------->x
*/
bool vector_xy_less_than(Coord (*a)[3], Coord (*b)[3], Plane p);

/* Note that !xy_greater_or_equal() is not equivalent to xy_less_than(),
   i.e. ax < bx && ay < by, as shown below:
  y
  |****####     # : values of a for which ax >= bx && ay >= by
  |****b###     * : values of a for which !(ax >= bx && ay >= by)
  |********         i.e. ax < bx || ay < by
 -|-------->x
*/
bool vector_xy_greater_or_equal(Coord (*a)[3],
                                Coord (*b)[3], const Plane p);

Coord vector_y_gradient(Coord (*a)[3], Coord (*b)[3], Plane p);

Coord vector_y_intercept(Coord (*a)[3], Coord m, Plane p);

bool vector_intersect(Coord (*va)[3], Coord (*vb)[3],
                      Coord (*vc)[3], Coord (*vd)[3],
                      Plane p, Coord (*intersect)[3]);

void vector_find_plane(Coord (*vector)[3], Plane *plane);

void vector_print(Coord (*a)[3]);

#endif /* VECTOR_H */
