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
  CJB: 26-Aug-18: Optimised calls to vector_x and vector_y.
                  Rewrote vector_xy_less_than to use coord_less_than.
                  Added a companion function, vector_xy_greater_or_equal.
  CJB: 17-Nov-18: vector_x/y/z are no longer inline functions.
 */

/* ISO library header files */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* Local header files */
#include "Vector.h"
#include "Coord.h"
#include "Internal/3dObjMisc.h"

Coord *vector_x(Coord (* const a)[3], Plane p)
{
  assert(a != NULL);
  assert(p.y != p.x);
  assert(p.z != p.x);
  assert(p.x < ARRAY_SIZE(*a));
  return &(*a)[p.x];
}

Coord *vector_y(Coord (* const a)[3], Plane p)
{
  assert(a != NULL);
  assert(p.x != p.y);
  assert(p.z != p.y);
  assert(p.y < ARRAY_SIZE(*a));
  return &(*a)[p.y];
}

 Coord *vector_z(Coord (* const a)[3], Plane p)
{
  assert(a != NULL);
  assert(p.x != p.z);
  assert(p.y != p.z);
  assert(p.z < ARRAY_SIZE(*a));
  return &(*a)[p.z];
}

void vector_mul(Coord (* const vector)[3],
                Coord const factor,
                Coord (* const product)[3])
{
  assert(vector != NULL);
  assert(product != NULL);
  assert(ARRAY_SIZE(*vector) == ARRAY_SIZE(*product));

  DEBUGF("{%"PCOORD",%"PCOORD",%"PCOORD"} * %"PCOORD" ",
         (*vector)[0], (*vector)[1], (*vector)[2], factor);
  for (size_t n = 0; n < ARRAY_SIZE(*vector); ++n) {
    (*product)[n] = (*vector)[n] * factor;
  }
  DEBUGF("= {%"PCOORD",%"PCOORD",%"PCOORD"}\n",
         (*product)[0], (*product)[1], (*product)[2]);
}

void vector_add(Coord (* const a)[3],
                Coord (* const b)[3],
                Coord (* const sum)[3])
{
  assert(a != NULL);
  assert(b != NULL);
  assert(sum != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*sum));

  DEBUGF("{%"PCOORD",%"PCOORD",%"PCOORD"} + "
         "{%"PCOORD",%"PCOORD",%"PCOORD"} ",
         (*a)[0], (*a)[1], (*a)[2],
         (*b)[0], (*b)[1], (*b)[2]);

  for (size_t n = 0; n < ARRAY_SIZE(*a); ++n) {
    (*sum)[n] = (*a)[n] + (*b)[n];
  }

  DEBUGF("= {%"PCOORD",%"PCOORD",%"PCOORD"}\n",
         (*sum)[0], (*sum)[1], (*sum)[2]);
}

void vector_sub(Coord (* const min)[3],
                Coord (* const sub)[3],
                Coord (* const diff)[3])
{
  assert(min != NULL);
  assert(sub != NULL);
  assert(diff != NULL);
  assert(ARRAY_SIZE(*min) == ARRAY_SIZE(*sub));
  assert(ARRAY_SIZE(*min) == ARRAY_SIZE(*diff));

  for (size_t n = 0; n < ARRAY_SIZE(*min); ++n) {
    (*diff)[n] = (*min)[n] - (*sub)[n];
  }
}

void vector_cross(Coord (* const a)[3],
                  Coord (* const b)[3],
                  Coord (* const prod)[3])
{
  assert(a != NULL);
  assert(b != NULL);
  assert(prod != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*prod));

  for (size_t n = 0; n < ARRAY_SIZE(*a); ++n) {
     (*prod)[n] = ((*a)[(n+1) % ARRAY_SIZE(*a)] * (*b)[(n+2) % ARRAY_SIZE(*a)]) -
                  ((*a)[(n+2) % ARRAY_SIZE(*a)] * (*b)[(n+1) % ARRAY_SIZE(*a)]);
  }
}

Coord vector_mag(Coord (* const a)[3])
{
  Coord mag = 0;
  assert(a != NULL);

  for (size_t n = 0; n < ARRAY_SIZE(*a); ++n) {
     mag += (*a)[n] * (*a)[n];
  }

  return coord_sqrt(mag);
}

bool vector_norm(Coord (* const a)[3], Coord (* const unit)[3])
{
  bool has_norm = false;

  assert(a != NULL);
  assert(unit != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*unit));

  const Coord mag = vector_mag(a);
  if (mag != 0) { /* no zero-length lines */
    for (size_t n = 0; n < ARRAY_SIZE(*a); ++n) {
      (*unit)[n] = (*a)[n] / mag;
    }
    has_norm = true;
  }
  return has_norm;
}

Coord vector_dot(Coord (* const a)[3], Coord (* const b)[3])
{
  Coord prod = 0;

  assert(a != NULL);
  assert(b != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));

  for (size_t n = 0; n < ARRAY_SIZE(*a); ++n) {
     prod += (*a)[n] * (*b)[n];
  }
  return prod;
}

bool vector_equal(Coord (* const a)[3], Coord (* const b)[3])
{
  bool is_eq = true;

  assert(a != NULL);
  assert(b != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));

  for (size_t n = 0; n < ARRAY_SIZE(*a) && is_eq; ++n) {
     if (!coord_equal((*a)[n], (*b)[n])) {
       is_eq = false;
     }
  }

  DEBUGF("{%"PCOORD",%"PCOORD",%"PCOORD"} %s "
         "{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
         (*a)[0], (*a)[1], (*a)[2],
         is_eq ? "==" : "!=",
         (*b)[0], (*b)[1], (*b)[2]);
  return is_eq;
}

bool vector_xy_less_than(Coord (* const a)[3], Coord (* const b)[3],
                         const Plane p)
{
  assert(a != NULL);
  assert(b != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));

  return coord_less_than(*vector_x(a, p), *vector_x(b, p)) &&
         coord_less_than(*vector_y(a, p), *vector_y(b, p));
}

bool vector_xy_greater_or_equal(Coord (* const a)[3], Coord (* const b)[3],
                                const Plane p)
{
  assert(a != NULL);
  assert(b != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));

  return !coord_less_than(*vector_x(a, p), *vector_x(b, p)) &&
         !coord_less_than(*vector_y(a, p), *vector_y(b, p));
}

Coord vector_y_gradient(Coord (* const a)[3],
                        Coord (* const b)[3],
                        const Plane p)
{
  assert(a != NULL);
  assert(b != NULL);
  assert(ARRAY_SIZE(*a) == ARRAY_SIZE(*b));

  const Coord ex = *vector_x(b, p) - *vector_x(a, p);
  const Coord ey = *vector_y(b, p) - *vector_y(a, p);
  assert(ex != 0); /* no vertical lines */
  const Coord m = ey / ex;
  DEBUGF("ex=%f ey=%f m=%f\n", ex, ey, m);
  return m;
}

Coord vector_y_intercept(Coord (* const a)[3], const Coord m, const Plane p)
{
  assert(a != NULL);
  const Coord x = *vector_x(a, p);
  const Coord y = *vector_y(a, p);
  const Coord c = y - (m * x);
  DEBUGF("x=%"PCOORD" y=%"PCOORD" m=%"PCOORD" c=%"PCOORD"\n",
         *vector_x(a, p), *vector_y(a, p), m, c);
  return c;
}

/* This function considers both lines AB and CD as infinite in length. */
bool vector_intersect(Coord (* const va)[3], Coord (* const vb)[3],
                      Coord (* const vc)[3], Coord (* const vd)[3],
                      const Plane p, Coord (* const intersect)[3])
{
  assert(va != NULL);
  assert(vb != NULL);
  assert(vc != NULL);
  assert(vd != NULL);
  assert(intersect != NULL);
  assert(va != vb);
  assert(vc != vd);

  DEBUGF("Finding intersection of A:{%"PCOORD",%"PCOORD",%"PCOORD"} .."
         " B:{%"PCOORD",%"PCOORD",%"PCOORD"} "
         "with C:{%"PCOORD",%"PCOORD",%"PCOORD"} .. "
         "D:{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
         (*va)[0], (*va)[1], (*va)[2],
         (*vb)[0], (*vb)[1], (*vb)[2],
         (*vc)[0], (*vc)[1], (*vc)[2],
         (*vd)[0], (*vd)[1], (*vd)[2]);

  Coord const ax = *vector_x(va, p), ay = *vector_y(va, p),
              bx = *vector_x(vb, p), by = *vector_y(vb, p),
              cx = *vector_x(vc, p), cy = *vector_y(vc, p),
              dx = *vector_x(vd, p), dy = *vector_y(vd, p);

  Coord ix, iy, iz;
  if (coord_equal(ax, bx)) {
    DEBUGF("line AB is vertical\n");
    /* If the lines overlap in the x dimension
       then there is only one place that they could cross. */
    ix = ax;

    if (coord_equal(cx, dx)) {
      DEBUGF("line CD is vertical\n");
      /* Both lines are vertical so they are parallel. */
      return false;
    }

    /* Compute the terms of the equation of line CD */
    const Coord m2 = vector_y_gradient(vc, vd, p);
    const Coord c2 = vector_y_intercept(vc, m2, p);

    /* Find y coordinate of the intersection with AB */
    iy = (m2 * ix) + c2;
  } else {
    /* AB is sloped or horizontal */
    if (coord_equal(ay, by)) {
      DEBUGF("line AB is horizontal\n");
      /* If the lines overlap in the y dimension
         then there is only one place that they could cross. */
      iy = ay;

      if (coord_equal(cx, dx)) {
        DEBUGF("line CD is vertical\n");
        /* If the lines overlap in the x dimension
           then there is only one place that they could cross. */
         ix = cx;
      } else {
        if (coord_equal(cy, dy)) {
          DEBUGF("line CD is horizontal\n");
           /* Both lines are horizontal so they are parallel. */
          return false;
        }
        /* Compute the terms of the equation of line CD */
        const Coord m2 = vector_y_gradient(vc, vd, p);
        const Coord c2 = vector_y_intercept(vc, m2, p);

        /* Find x coordinate of the intersection with AB:
           y = mx + c
           mx = y - c
           x = (y - c)/m */
        ix = (iy-c2)/m2;
      }
    } else {
      /* AB is neither vertical nor horizontal.
         Compute the terms of the equation of line AB */
      const Coord m1 = vector_y_gradient(va, vb, p);
      const Coord c1 = vector_y_intercept(va, m1, p);

      if (coord_equal(cx, dx)) {
        DEBUGF("line CD is vertical\n");
        /* If the lines overlap in the x dimension then there is only one place
           that they could cross. */
        ix = cx;
      } else {
         /* Compute the terms of the equation of line CD */
        const Coord m2 = vector_y_gradient(vc, vd, p);
        if (coord_equal(m1, m2)) {
          DEBUGF("lines CD and AB are parallel\n");
          return false;
        }
        const Coord c2 = vector_y_intercept(vc, m2, p);

        /* Find x coordinate of the intersection with AB
           (where the two edges have equal y):
           (m1 * x) + c1 = (m2 * x) + c2
           (m1 * x) - (m2 * x) = c2 - c1
           (m1 - m2) * x = c2 - c1 */
        ix = (c2 - c1) / (m1 - m2);
      }

      /* Find y coordinate of the intersection with CD */
      iy = (m1 * ix) + c1;
    }
  }

  /* Compute the terms of the equation of line AB in the xz plane */
  Plane p2 = { p.x, p.z, p.y };
  if (coord_equal(ax, bx)) {
    /* If the two lines are parallel in the xy plane then
       we should have given up before now. */
    assert(cx != dx);

    /* Compute the terms of the equation of line CD */
    const Coord m3 = vector_y_gradient(vc, vd, p2);
    const Coord c3 = vector_y_intercept(vc, m3, p2);

    /* Find z coordinate of the intersection with AB */
    iz = (m3 * ix) + c3;

  } else {
    const Coord m4 = vector_y_gradient(va, vb, p2);
    const Coord c4 = vector_y_intercept(va, m4, p2);

    /* Find z coordinate of the intersection with CD */
    iz = (m4 * ix) + c4;
  }

  *vector_x(intersect, p) = ix;
  *vector_y(intersect, p) = iy;
  *vector_z(intersect, p) = iz;
  DEBUGF("Intersection is at {%"PCOORD",%"PCOORD",%"PCOORD"}\n",
         (*intersect)[0], (*intersect)[1], (*intersect)[2]);

  return true;
}

void vector_find_plane(Coord (* const vector)[3], Plane * const plane)
{
  assert(vector != NULL);
  assert(plane != NULL);

  /* Find the dimension with the biggest size */
  size_t bd = 0;
  Coord biggest = -COORD_INF;
  for (size_t dim = 0; dim < ARRAY_SIZE(*vector); ++dim) {
    DEBUGF("Range in dimension %zu is %"PCOORD"\n",
           dim, (*vector)[dim]);
    const Coord mag = coord_abs((*vector)[dim]);
    if (mag > biggest) {
      biggest = mag;
      bd = dim;
    }
  }

  DEBUGF("Biggest range %"PCOORD" is dimension %zu\n", biggest, bd);
  plane->x = (0 == bd ? 2 : 0);
  plane->y = (1 == bd ? 2 : 1);
  /* We'll ignore the z dimension when projecting the plane into two
     dimensions so set it to the biggest component of the plane's normal
     vector. */
  plane->z = bd;
}

void vector_print(Coord (* const a)[3])
{
  assert(a != NULL);
  printf("{%"PCOORD",%"PCOORD",%"PCOORD"}", (*a)[0], (*a)[1], (*a)[2]);
}
