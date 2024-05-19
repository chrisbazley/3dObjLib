/*
 * 3dObjLib: Geometric primitive storage
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
  CJB: 26-Aug-18: Deleted primitive_contains_point, primitive_get_top_y
                  and primitive_get_bbox because they aren't used outside
                  this module.
                  Added a macro to allow bounding box optimisations to be
                  disabled.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
 */

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <stdint.h>
#include <stdbool.h>

#include "Vertex.h"

/* Setting this switch to 1 must make processing more efficient without
   changing the output. Timings for processing the 'Warrior' graphics set
   belonging to Star Fighter 3000 are as follows:
     BBOX=0: 5.24 seconds
     BBOX=1: 2.66 seconds (roughly double speed)
*/
#define BBOX 1

typedef struct {
  int colour;
  int id;
  int nsides;
  int sides[15];
  Coord normal[3];
  bool has_normal;
#if BBOX
  bool has_bbox;
  Coord low[3];
  Coord high[3];
#endif
} Primitive;

void primitive_init(Primitive *primitive);

int primitive_get_side(const Primitive *primitive, int n);

int primitive_add_side(Primitive *primitive, int v);

void primitive_delete_all(Primitive *primitive);

void primitive_reverse_sides(Primitive *primitive);

bool primitive_get_normal(Primitive *primitive,
                          const VertexArray *varray,
                          Coord (*norm)[3]);

bool primitive_set_normal(Primitive *primitive,
                          const VertexArray *varray,
                          Coord (*norm)[3]);

void primitive_set_colour(Primitive *primitive, int colour);

int primitive_get_colour(const Primitive *primitive);

int primitive_get_num_sides(const Primitive *primitive);

int primitive_get_id(const Primitive *primitive);

void primitive_set_id(Primitive *primitive, int id);

bool primitive_coplanar(Primitive *p, Primitive *q,
                        const VertexArray *varray);

bool primitive_find_plane(Primitive *primitive,
                          const VertexArray *varray, Plane *plane);

bool primitive_contains(Primitive *q, Primitive *p,
                        const VertexArray *varray, Plane plane);

bool primitive_equal(const Primitive *q, const Primitive *p);

bool primitive_intersect(const Primitive *primitive,
                         int a, int b,
                         const VertexArray *varray, Plane plane);

bool primitive_split(Primitive *primitive, int a, int b,
                     VertexArray *varray, Plane plane,
                     Primitive *out, bool *split);

bool primitive_clip(Primitive *primitive,
                    Primitive *clipper,
                    VertexArray *varray, Plane plane,
                    Primitive *out, bool *split);

void primitive_set_used(const Primitive *primitive,
                        const VertexArray *varray);

int primitive_get_skew_side(Primitive *primitive,
                            const VertexArray *varray);

void primitive_print(const Primitive *primitive,
                     const VertexArray *varray);

#endif /* PRIMITIVE_H */
