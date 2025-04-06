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
  CJB: 26-Aug-18: Fixed a bug in the clipping algorithm: primitive_intersect
                  must not ignore intersections between the given edge and
                  a polygon which are coincident with the polygon's corners.
                  Fixed a bug where primitive_delete_all omitted to reset
                  the flags concerning cached normal and bounding box.
                  Optimised primitive_contains, primitive_contains_point and
                  primitive_get_top_y by using the cached bounding box.
                  primitive_contains_point now returns early if the input
                  vertex is part of the definition of the polygon.
                  primitive_contains now returns early if any vertex of one
                  polygon is found to be outside the other.
                  primitive_equal and primitive_intersect now safely handle
                  the case where a primitive has no sides.
                  Made primitive_contains_point and primitive_get_top_y
                  static because they aren't used outside this module.
                  Created variants of primitive_get_normal and
                  primitive_get_bbox for use when the result doesn't need to
                  be copied.
                  Added a macro to allow bounding box optimisations to be
                  disabled (mainly for debugging).
  CJB: 03-Nov-18: Simplified primitive_get_top_y to work around a bug in the
                  Norcroft C compiler.
  CJB: 09-Jan-21: Initialize struct using compound literal assignment to
                  guard against leaving members uninitialized.
  CJB: 30-Jul-22: Extra range check in primitive_get_side to stop a wrong
                  warning from GCC's -Warray-bounds.
  CJB: 06-Apr-25: Dogfooding the _Optional qualifier.
 */

/* ISO library header files */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* Local header files */
#include "Primitive.h"
#include "Vertex.h"
#include "Vector.h"
#include "Coord.h"
#include "Internal/3dObjMisc.h"

void primitive_init(Primitive * const primitive)
{
  assert(primitive != NULL);
  *primitive = (Primitive){
    .colour = 0,
    .id = 0,
    .nsides = 0,
    .has_normal = false,
#if BBOX
    .has_bbox = false,
#endif
  };
}

int primitive_get_side(const Primitive * const primitive, const int n)
{
  int side = -1;

  assert(primitive != NULL);
  assert(primitive->nsides <= (int)ARRAY_SIZE(primitive->sides));

  if ((n >= 0) && (n < primitive->nsides) &&
      (n < (int)ARRAY_SIZE(primitive->sides))) {
    side = primitive->sides[n];
  } else {
    DEBUGF("Invalid side number %d\n", n);
  }

  return side;
}

int primitive_add_side(Primitive * const primitive, const int v)
{
  int side = -1;

  assert(primitive != NULL);
  assert(primitive->nsides <= (int)ARRAY_SIZE(primitive->sides));

  if (v >= 0) {
    if ((size_t)primitive->nsides + 1 <= ARRAY_SIZE(primitive->sides)) {
      side = primitive->nsides++;
      primitive->sides[side] = v;
      primitive->has_normal = false;
#if BBOX
      primitive->has_bbox = false;
#endif
      DEBUGF("Set side %d of primitive %p to vertex %d\n",
              primitive->nsides, (void *)primitive, v);

    } else {
      DEBUGF("Primitive has more than %zu sides\n",
             ARRAY_SIZE(primitive->sides));
    }
  } else {
    DEBUGF("Invalid vertex number %d\n", v);
  }
  return side;
}

void primitive_delete_all(Primitive * const primitive)
{
  DEBUGF("Deleting %d sides of primitive %p\n",
         primitive->nsides, (void *)primitive);
  primitive->nsides = 0;
  primitive->has_normal = false;
#if BBOX
  primitive->has_bbox = false;
#endif
}

void primitive_reverse_sides(Primitive * const primitive)
{
  assert(primitive != NULL);
  DEBUGF("Reversing %d sides of primitive %p\n",
         primitive->nsides, (void *)primitive);

  const int mid = primitive->nsides/2;
  for (int low = 0; low < mid; ++low) {
    const int temp = primitive->sides[low];
    const int high = (primitive->nsides - low) - 1;
    primitive->sides[low] = primitive->sides[high];
    primitive->sides[high] = temp;
  }
  primitive->has_normal = false;
}

static bool primitive_make_normal(Primitive * const primitive,
                                  const VertexArray * const varray,
                                  Coord (*const norm)[3])
{
  bool has_normal = false;

  assert(primitive != NULL);
  assert(norm != NULL);

  int const nsides = primitive_get_num_sides(primitive);
  if (nsides < 3) {
    DEBUGF("Primitive %p with %d sides can't have a normal\n",
           (void *)primitive, nsides);
  } else {
    /* Compute the polygon's normal vector, which is orthogonal to it.
       This is useful for determining its coplanarity with respect to other
       polygons, among other uses. */
    Coord side_one[3], side_two[3], cross_prod[3];
    Coord (*coords[3])[3];

    for (int side = 0; side < 3; ++side) {
      const int v = primitive_get_side(primitive, side);
      _Optional Coord (*c)[3] = vertex_array_get_coords(varray, v);
      if (!c) {
        return false;
      }
      coords[side] = &*c;
    }

    vector_sub(coords[1], coords[0], &side_one);
    vector_sub(coords[2], coords[1], &side_two);
    vector_cross(&side_one, &side_two, &cross_prod);
    has_normal = vector_norm(&cross_prod, norm);
  }

  if (has_normal) {
    DEBUGF("Normal of primitive %p is {%"PCOORD",%"PCOORD",%"PCOORD"}\n",
           (void *)primitive, (*norm)[0], (*norm)[1], (*norm)[2]);
  } else {
    DEBUGF("Cannot compute normal for primitive %p\n", (void *)primitive);
  }
  return has_normal;
}

static bool primitive_ensure_normal(Primitive * const primitive,
                                    const VertexArray * const varray)
{
  assert(primitive != NULL);
  if (!primitive->has_normal) {
    if (primitive_make_normal(primitive, varray, &primitive->normal)) {
      primitive->has_normal = true;
    }
  }
  return primitive->has_normal;
}

bool primitive_get_normal(Primitive * const primitive,
                          const VertexArray * const varray,
                          Coord (* const norm)[3])
{
  assert(primitive != NULL);
  assert(norm != NULL);

  if (primitive_ensure_normal(primitive, varray)) {
    for (size_t n = 0; n < ARRAY_SIZE(*norm); ++n) {
      (*norm)[n] = primitive->normal[n];
    }
    return true;
  }

  return false;
}

bool primitive_set_normal(Primitive * const primitive,
                          const VertexArray * const varray,
                          Coord (* const norm)[3])
{
  bool reversed = false;

  if (primitive_ensure_normal(primitive, varray)) {
    if (!vector_equal(norm, &primitive->normal)) {
      primitive_reverse_sides(primitive);
      assert(primitive_ensure_normal(primitive, varray));
      assert(vector_equal(norm, &primitive->normal));
      reversed = true;
    }
  }
  return reversed;
}

void primitive_set_colour(Primitive * const primitive, const int colour)
{
  assert(primitive != NULL);
  assert(colour >= 0);
  DEBUGF("Setting colour of primitive %p to %d (0x%x)\n", (void *)primitive,
         colour, colour);
  primitive->colour = colour;
}

int primitive_get_colour(const Primitive * const primitive)
{
  assert(primitive != NULL);
  return primitive->colour;
}

int primitive_get_num_sides(const Primitive * const primitive)
{
  assert(primitive != NULL);
  assert((size_t)primitive->nsides <= ARRAY_SIZE(primitive->sides));
  return primitive->nsides;
}

int primitive_get_id(const Primitive * const primitive)
{
  assert(primitive != NULL);
  assert(primitive->id >= 0);
  return primitive->id;
}

void primitive_set_id(Primitive * const primitive, int id)
{
  assert(primitive != NULL);
  if (id >= 0) {
    primitive->id = id;
  } else {
    DEBUGF("Invalid primitive ID %d\n", id);
  }
}

#if BBOX
static bool primitive_make_bbox(Primitive * const primitive,
                                const VertexArray * const varray,
                                Coord (*const low)[3],
                                Coord (*const high)[3])
{
  bool has_bbox = false;

  assert(primitive != NULL);
  assert(low != NULL);
  assert(high != NULL);

  const int nsides = primitive_get_num_sides(primitive);
  if (nsides < 1) {
    DEBUGF("Primitive %p with %d sides can't have a bounding box\n",
           (void *)primitive, nsides);
  } else {
    /* Compute the smallest cuboid region containing all vertices of
       the primitive. */
    const int p0 = primitive_get_side(primitive, 0);
    _Optional Coord (* const coords0)[3] = vertex_array_get_coords(varray, p0);
    if (!coords0) {
      return false;
    }
    for (size_t dim = 0; dim < ARRAY_SIZE(*coords0); ++dim) {
      (*low)[dim] = (*high)[dim] = (*coords0)[dim];
    }

    for (int s = 1; s < nsides; ++s) {
      const int pv = primitive_get_side(primitive, s);
      _Optional Coord (* const coords)[3] = vertex_array_get_coords(varray, pv);
      if (!coords) {
        return false;
      }

      for (size_t dim = 0; dim < ARRAY_SIZE(*coords); ++dim) {
        if ((*coords)[dim] > (*high)[dim]) {
          (*high)[dim] = (*coords)[dim];
        }
        if ((*coords)[dim] < (*low)[dim]) {
          (*low)[dim] = (*coords)[dim];
        }
      }
    }
    has_bbox = true;
  }

  if (has_bbox) {
    DEBUGF("Primitive %p has bbox "
           "{%"PCOORD",%"PCOORD",%"PCOORD"},"
           "{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
           (void *)primitive,
           (*low)[0], (*low)[1], (*low)[2],
           (*high)[0], (*high)[1], (*high)[2]);
  } else {
    DEBUGF("Cannot make bbox for primitive %p\n", (void *)primitive);
  }
  return has_bbox;
}

static bool primitive_ensure_bbox(Primitive * const primitive,
                                  const VertexArray * const varray)
{
  assert(primitive != NULL);
  if (!primitive->has_bbox) {
    if (primitive_make_bbox(primitive, varray,
                            &primitive->low, &primitive->high)) {
      primitive->has_bbox = true;
    }
  }
  return primitive->has_bbox;
}
#endif /* BBOX */

bool primitive_find_plane(Primitive * const primitive,
                          const VertexArray * const varray,
                          Plane * const plane)
{
  const bool has_normal = primitive_ensure_normal(primitive, varray);
  if (has_normal) {
    vector_find_plane(&primitive->normal, plane);
  }
  return has_normal;
}

bool primitive_coplanar(Primitive *p, Primitive *q,
                        const VertexArray * const varray)
{
  assert(p != NULL);
  assert(q != NULL);

  bool const got_p = primitive_ensure_normal(p, varray);
  if (!got_p) {
    DEBUGF("No normal for primitive %p\n", (void *)p);
  }

  bool const got_q = primitive_ensure_normal(q, varray);
  if (!got_q) {
    DEBUGF("No normal for primitive %p\n", (void *)q);
  }

  bool coplanar = false;
  if (got_p || got_q) {
    Coord (*norm)[3] = &p->normal;
    int nsides_q = 1;
    coplanar = true;

    if (got_p && got_q) {
      /* Two polygons cannot be coplanar unless they have the same
         normal vector. */
      if (!vector_equal(&p->normal, &q->normal)) {
        DEBUGF("Primitives %p and %p have different normals\n",
               (void *)p, (void *)q);
        coplanar = false;
      } else {
        /* We only need to check the angle of one vertex of q relative to the
           normal vector of p if both polygons are facing the same direction. */
        nsides_q = 1;
      }
    } else {
      /* We can also compute coplanarity given only one normal vector.
         The above test fails for two lines, two points, or a point and a line. */

      /* Swap p and q if necessary to ensure that p has a normal vector. */
      if (!got_p) {
        Primitive * const tmp = p;
        p = q;
        q = tmp;
        assert(got_q);
        norm = &q->normal;
      }

      /* We need to check every vertex of q individually if we cannot prove
         it is facing the same direction as p. */
      nsides_q = primitive_get_num_sides(q);
    }
    int const vp = primitive_get_side(p, 0);

    /* Check each vertex of q for coplanarity until we find one in a
       different plane from p (or skip this loop if p and q are polygons
       facing different directions -- see above). */
    for (int s = 0; coplanar && (s < nsides_q); ++s) {
      int const vq = primitive_get_side(q, s);

      /* Find whether each vertex of q is in the same plane as polygon p by
         projecting a vector between that vertex and the first vertex of p
         onto the normal vector of p to get the shortest straight-line distance
         between that vertex of q and any point on the plane containing p:

                 |
             norm|     vq,_____q_____
                 | diff,/|
                 |   ,/  |dot(norm,diff)
           ____p_|__/vp  |

         The dot product of the normal and a vector between the polygon and
         vertex is 0 if the two vectors are orthogonal, which only occurs if
         the vertex is in the same plane as the polygon. */
      Coord diff[3];
      _Optional Coord (*pcoords)[3] = vertex_array_get_coords(varray, vp),
                      (*qcoords)[3] = vertex_array_get_coords(varray, vq);
      if (!pcoords || !qcoords) {
        return false;
      }

      vector_sub(&*pcoords, &*qcoords, &diff);

      const Coord dist = coord_abs(vector_dot(norm, &diff));

      DEBUGF("Projected {%"PCOORD",%"PCOORD",%"PCOORD"} "
             "onto {%"PCOORD",%"PCOORD",%"PCOORD"} "
             "to get distance %"PCOORD"\n",
             diff[0], diff[1], diff[2],
             (*norm)[0], (*norm)[1], (*norm)[2],
             dist);

      coplanar = coord_equal(dist, 0);
      DEBUGF("Vertex %d is%s coplanar with polygon %p\n", vq,
             coplanar ? "" : " not", (void *)p);
    }
  }

  DEBUGF("Primitives %p and %p are%s coplanar\n", (void *)p, (void *)q,
         coplanar ? "" : " not");
  return coplanar;
}

/* Find the highest coordinate of the primitive. */
static Coord primitive_get_top_y(Primitive * const primitive,
                                 const VertexArray *varray,
                                 const Plane plane)
{
  assert(primitive != NULL);
#if BBOX
  assert(primitive->has_bbox);
  Coord const top_y = primitive->high[plane.y];
  NOT_USED(varray);
#else /* BBOX */
  Coord top_y = -COORD_INF;
  const int nsides = primitive_get_num_sides(primitive);
  for (int s = 0; s < nsides; ++s) {
    const int v = primitive_get_side(primitive, s);
    Coord (* const coords)[3] = vertex_array_get_coords(varray, v);
    const Plane p = plane;
    Coord const y = *vector_y(coords, p);
    if (y > top_y) {
      top_y = y;
    }
  }
#endif /* BBOX */
  DEBUGF("Top y is %"PCOORD"\n", top_y);
  return top_y;
}

/* This implementation allows for floating-point error and assumes that
   nearby points are contained within a polygon. This is important because
   it is used to decide which half of a split polygon to delete. */
static bool primitive_contains_point(Primitive * const primitive,
                                     const VertexArray * const varray,
                                     const int v, const Plane plane)
{
  bool is_inside = false;

  assert(primitive != NULL);

  const int nsides = primitive_get_num_sides(primitive);
  if (nsides < 3) {
    DEBUGF("Primitive %p with %d sides can't contain point %d\n",
           (void *)primitive, nsides, v);
    return false;
  }

  _Optional Coord (*start)[3];
  Coord start_x, start_y;

  const int last_side = primitive_get_side(primitive, nsides - 1);
  if (last_side == v) {
    DEBUGF("Point %d is also the end of the last edge\n", v);
    return true;
  }

  _Optional Coord (*end)[3] = vertex_array_get_coords(varray, last_side);
  if (!end) {
    return false;
  }
  Coord end_x = *vector_x(&*end, plane);
  Coord end_y = *vector_y(&*end, plane);

  _Optional Coord (* const point)[3] = vertex_array_get_coords(varray, v);
  if (!point) {
    return false;
  }
  const Coord px = *vector_x(&*point, plane);
  Coord py = *vector_y(point, plane);

#if BBOX
  /* If the point is outside the bounding box (even allowing for error)
     then it can't be inside the polygon. */
  assert(primitive->has_bbox);
  if (!vector_xy_greater_or_equal(&*point, &primitive->low, plane) ||
      !vector_xy_greater_or_equal(&primitive->high, &*point, plane)) {
    DEBUGF("Point %d is outside bounding box of primitive %p\n",
           v, (void *)primitive);
    return false;
  }
#endif /* BBOX */

  Coord const top_y = primitive_get_top_y(primitive, varray, plane);

  for (int s = 0;
       s < nsides; ++s,
       end = start, end_x = start_x, end_y = start_y) {

    const int v2 = primitive_get_side(primitive, s);
    if (v2 == v) {
      DEBUGF("Point %d is also the start of edge %d\n", v, s);
      return true;
    }

    start = vertex_array_get_coords(varray, v2);
    if (!start) {
      return false;
    }
    start_x = *vector_x(&*start, plane);
    start_y = *vector_y(&*start, plane);
    assert(end_x == *vector_x(&*end, plane));
    assert(end_y == *vector_y(&*end, plane));

    DEBUGF("Testing point %d:%"PCOORD",%"PCOORD" against edge %d:"
           "%"PCOORD",%"PCOORD" .. %"PCOORD",%"PCOORD"\n",
            v, px, py, s, start_x, start_y, end_x, end_y);

    /* Select edges that might be in the path of a ray from the point
       to be tested to infinite +x. */

    /* Ignore edges left of the point to be tested. */
    const Coord high_x = HIGHEST(start_x, end_x);
    if (coord_less_than(high_x, px)) {
      continue;
    }

    /* Treat horizontal edges specially to avoid division by 0: */
    if (coord_equal(end_y, start_y)) {
      /* Ignore horizontal edges right of the point to be tested. */
      const Coord low_x = LOWEST(start_x, end_x);
      if (coord_less_than(px, low_x)) {
        continue;
      }
      /* Horizontal edge overlaps the point in the x dimension. */

      if (coord_equal(py, end_y) || coord_equal(py, start_y)) {
        /* Horizontal edge intersects the point at its y coordinate. */
        DEBUGF("Point %d is coincident with horizontal edge %d\n", v, s);
        return true;
      }

      continue;
    }

    /* Be precise about the y-extent of edges to ensure that x-intersections
       with the polygon are actually inside it (unlike 'a' below):
         a
          \/
          /\
        _/__\_
        /    \
     */

    /* Ignore edges above the point to be tested. */
    const Coord low_y = LOWEST(start_y, end_y);
    if (py < low_y) {
      continue;
    }

    /* Ignore edges below the point to be tested. */
    const Coord high_y = HIGHEST(start_y, end_y);
    if (py > high_y) {
      continue;
    }

    /* Exclude the highest endpoint of each edge except the top one(s)
       to avoid erroneously recording two crossings at one corner. */
    if ((py == high_y) && (high_y != top_y)) {
      continue;
    }

    /* Find the x coordinate at which the horizontal ray intersects the
       edge. */
    Coord intersect_x;
    if (coord_equal(end_x, start_x)) {
      /* Vertical edge intersects the horizontal ray at its x coordinate. */
      intersect_x = start_x;
    } else {
      /* Sloping edge intersects the horizontal ray somewhere
          along its length. */
      const Coord m = vector_y_gradient(&*start, &*end, plane);
       /* The equation of any line is y=mx+c.
          This can be rearranged as c=y-mx. Since we know m, we can
          can substitute the coordinates of any point on the line into
          the equation as x and y in order to find c. Let's use the
          coordinates of an endpoint of the edge and call them s,t.
          y=mx+t-ms
          mx=y-t+ms
          x=(y-t+ms)/m
          x=s+((y-t)/m)
          Now substitute the y coordinate of the horizontal ray into
          the above equation to find the x coordinate of the point
          where it intersects the edge. */
       intersect_x = start_x + ((py - start_y) / m);
    }

    /* Unfortunately an inexact comparison here allows more leeway for
       points near steep lines than shallow ones. */
    if (coord_equal(px, intersect_x)) {
      DEBUGF("Point %d is coincident with edge %d\n", v, s);
      return true;
    }

    if (coord_less_than(px, intersect_x)) {
      DEBUGF("%s\n", is_inside ? "Inside to outside" : "Outside to inside");
      is_inside = !is_inside;
    }
  }

  return is_inside;
}

bool primitive_contains(Primitive * const q, Primitive * const p,
                        const VertexArray * const varray, const Plane plane)
{
#if BBOX
  /* Get the smallest cuboids containing the two primitives. */
  if (!primitive_ensure_bbox(q, varray) ||
      !primitive_ensure_bbox(p, varray)) {
    DEBUGF("Can't determine nesting using incomplete primitive\n");
    return false;
  }

  /* The bounding box of q must include that of p. */
  if (!vector_xy_greater_or_equal(&p->low, &q->low, plane) ||
      !vector_xy_greater_or_equal(&q->high, &p->high, plane)) {
    DEBUGF("Primitive %p's bbox does not cover %p\n",
           (void *)q, (void *)p);
    return false;
  }
#endif /* BBOX */

  /* Check for any vertices of primitive P lying within primitive Q. */
  const int nsides_p = primitive_get_num_sides(p);
  for (int t = 0; t < nsides_p; ++t) {
    const int side_p = primitive_get_side(p, t);
    if (!primitive_contains_point(q, varray, side_p, plane)) {
      DEBUGF("Primitive %p does not contain side %d (vertex %d) "
             "of primitive %p\n", (void *)q, t, side_p, (void *)p);
      return false;
    }
  }

  DEBUGF("All %d sides of primitive %p are contained by primitive %p\n",
          nsides_p, (void *)p, (void *)q);
  return true;
}

bool primitive_equal(const Primitive * const q, const Primitive * const p)
{
  /* If the number of sides differs then the primitives can't be equal. */
  const int nsides_q = primitive_get_num_sides(q);
  const int nsides_p = primitive_get_num_sides(p);
  if (nsides_p != nsides_q) {
    DEBUGF("Primitives %p and %p have a different no. of sides (%d,%d)\n",
            (void *)p, (void *)q, nsides_p, nsides_q);
    return false;
  }

  if (nsides_p > 0) {
    /* Search for the first vertex of P in Q. */
    const int first_side_p = primitive_get_side(p, 0);
    bool found = false;
    int s;
    for (s = 0; !found && (s < nsides_q); ++s) {
      const int side_q = primitive_get_side(q, s);
      if (side_q == first_side_p) {
        DEBUGF("Found first vertex %d of primitive %p as side %d/%d of %p\n",
                first_side_p, (void *)p, s, nsides_q, (void *)q);
        found = true;
      }
    }

    if (!found) {
      DEBUGF("First vertex %d of primitive %p is not in %p\n",
              first_side_p, (void *)p, (void *)q);
      return false;
    }

    /* Check that the following vertices are the same in P and Q. */
    for (int t = 1 /* intentional */; t < nsides_p; ++t, ++s) {
      const int side_p = primitive_get_side(p, t);
      if (s >= nsides_q) {
        s = 0;
      }
      const int side_q = primitive_get_side(q, s);
      if (side_q != side_p) {
        DEBUGF("Side %d/%d (%d) of primitive %p mismatches "
               "side %d/%d (%d) of %p\n",
               t, nsides_p, side_p, (void *)p,
               s, nsides_q, side_q, (void *)q);
        return false;
      }
    }
  }

  DEBUGF("Primitives %p and %p are equal\n", (void *)p, (void *)q);
  return true;
}

bool primitive_intersect(const Primitive * const primitive,
                         const int a, const int b,
                         const VertexArray * const varray,
                         const Plane plane)
{
  const int nsides = primitive_get_num_sides(primitive);
  if (nsides < 3) {
    /* We might be able to handle this for lines and points in future
       but there's currently no need. */
    DEBUGF("Primitive %p with %d sides can't intersect with edge %d,%d\n",
           (void *)primitive, nsides, a, b);
  } else {
    int last_side = primitive_get_side(primitive, nsides-1);
    /* Use last_side twice: first as the start of an edge and last as the end
       of an edge. */
    for (int s = 0; s < nsides; ++s) {
      const int side = primitive_get_side(primitive, s);

      /* Shared vertices don't count. */
      if ((a == last_side) || (b == last_side) ||
          (a == side) || (b == side)) {
        DEBUGF("Edge %d .. %d is joined with line %d .. %d "
               "(shared vertex)\n", a, b, last_side, side);
      } else {
        Coord intersect[3];
        if (vertex_array_edges_intersect(varray, a, b, last_side, side,
                                         plane, &intersect)) {

          /* Treat the endpoints of the given edge of the front polygon as
             exclusive to avoid treating contiguous polygons as overlapping.
             We cannot treat any endpoints of the back polygon's edges as
             exclusive because it's common for a back polygon to be split by
             a line that happens to pass through one of its corners. */
          _Optional Coord (*const acoords)[3] = vertex_array_get_coords(varray, a);
          if (!acoords) {
            return false;
          }
          if (vector_equal(&intersect, &*acoords)) {
            DEBUGF("Edge %d .. %d is joined with line %d .. %d "
                   "(at vertex %d)\n", a, b, last_side, side, a);
          } else {
            _Optional Coord (*const bcoords)[3] = vertex_array_get_coords(varray, b);
            if (!bcoords) {
              return false;
            }
            if (vector_equal(&intersect, &*bcoords)) {
              DEBUGF("Edge %d .. %d is joined with line %d .. %d "
                    "(at vertex %d)\n", a, b, last_side, side, b);
            } else {
              DEBUGF("Side %d (%d) of primitive %p intersects edge %d,%d\n",
                      s, side, (void *)primitive, a, b);
              return true;
            }
          }
        }
      }
      last_side = side;
    }
  }

  DEBUGF("Primitive %p and edge %d,%d do not intersect\n",
         (void *)primitive, a, b);
  return false;
}

bool primitive_split(Primitive * const primitive, const int a, const int b,
                     VertexArray * const varray, const Plane plane,
                     Primitive * const out, bool * const split)
{
  assert(out != NULL);
  assert(split != NULL);

  Primitive tmp;
  primitive_init(&tmp);
  enum {
    SPLIT_NONE,
    SPLIT_IN_PROGRESS,
    SPLIT_COMPLETE
  } state = SPLIT_NONE;

  const int num_sides = primitive_get_num_sides(primitive);
  if (num_sides < 3) {
    DEBUGF("Can't split primitive %p with %d sides\n",
           (void *)primitive, num_sides);
  } else {
    int last_side = primitive_get_side(primitive, num_sides-1);

    for (int s = 0; s < num_sides; ++s) {
      const int side = primitive_get_side(primitive, s);
      DEBUGF("Back side %d/%d: %d\n", s, num_sides, side);

      Coord intersect[3];
      if ((state != SPLIT_COMPLETE) &&
          vertex_array_edge_intersects_line(varray, last_side, side,
                                            a, b, plane, &intersect))
      {
        DEBUGF("Splitting edge %d .. %d with line %d .. %d\n",
                last_side, side, a, b);

        int v = vertex_array_find_vertex(varray, &intersect);
        if (v < 0) {
          v = vertex_array_add_vertex(varray, &intersect);
          if (v < 0) {
            return false;
          }
        }

        if (state == SPLIT_IN_PROGRESS) {
          DEBUGF("Finishing clip\n");
          state = SPLIT_COMPLETE;

          if ((v != last_side) && primitive_add_side(out, v) < 0) {
            return false;
          }

          /* Also restart polygon P at the point of intersection if it
             differs from the current vertex (which we're about to keep
             as part of P anyhow). Careful because this can increase the
             number of sides in P. */
          if (v != side) {
            if (primitive_add_side(&tmp, v) < 0) {
              return false;
            }
          }

        } else {
          DEBUGF("Starting clip\n");
          state = SPLIT_IN_PROGRESS;

          /* Clip the original polygon at the point of intersection
             (which may simply be the previous vertex). */
          if (v != last_side) {
            if (primitive_add_side(&tmp, v) < 0) {
              return false;
            }
          }

          /* Begin a new polygon at the point of intersection. */
          primitive_init(out);
          if ((v != side) && primitive_add_side(out, v) < 0) {
            return false;
          }
        }
      }

      /* If we're clipping the original polygon then move vertices between
         the two edge intersections to the new polygon. */
      if (primitive_add_side(state == SPLIT_IN_PROGRESS ? out : &tmp, side) < 0) {
        return false;
      }
      last_side = side;
    }
  }

  if (state == SPLIT_COMPLETE) {
    *split = true;

    assert(primitive_get_num_sides(&tmp) > 2);
    assert(primitive_coplanar(&tmp, primitive, varray));
    assert(primitive_get_num_sides(out) > 2);
    assert(primitive_coplanar(out, primitive, varray));

    primitive_delete_all(primitive);

    /* Copy sides data from the temporary polygon to the input polygon. */
    const int tmp_sides = primitive_get_num_sides(&tmp);
    DEBUGF("Copying %d sides of primitive %p to %p\n",
           tmp_sides, (void *)&tmp, (void *)primitive);

    for (int s = 0; s < tmp_sides; ++s) {
      const int side = primitive_get_side(&tmp, s);
      if (primitive_add_side(primitive, side) < 0) {
        return false;
      }
    }

    /* Copy other data from the input polygon to the new polygon */
    DEBUGF("Finishing new primitive %p\n", (void *)out);
    primitive_set_colour(out, primitive_get_colour(primitive));
    primitive_set_id(out, primitive_get_id(primitive));

    if (primitive->has_normal) {
      for (size_t n = 0; n < ARRAY_SIZE(out->normal); ++n) {
        out->normal[n] = primitive->normal[n];
      }
      out->has_normal = true;
    }
  } else {
    *split = false;
  }

  return true;
}

bool primitive_clip(Primitive * const primitive,
                    Primitive * const clipper,
                    VertexArray * const varray, const Plane plane,
                    Primitive * const out, bool * const split)
{
  assert(clipper != NULL);
  assert(clipper != primitive);
  assert(split != NULL);

#if BBOX
  /* Get the smallest cuboids containing the two primitives. */
  if (!primitive_ensure_bbox(clipper, varray) ||
      !primitive_ensure_bbox(primitive, varray)) {
    DEBUGF("Can't clip using incomplete primitive\n");
    return false;
  }

  /* If the rectangles don't overlap then the actual polygons don't
     overlap either. */
  if (!vector_xy_less_than(&clipper->low, &primitive->high, plane) ||
      !vector_xy_less_than(&primitive->low, &clipper->high, plane)) {
    DEBUGF("Primitive bboxes do not overlap\n");
    return true;
  }
#endif /* BBOX */

  /* Consider each edge of the front primitive individually as a potential
     subdividing line. Stop after dividing the back polygon in two to
     re-evaluate which polygons are occluded. */
  const int num_sides = primitive_get_num_sides(clipper);
  if (num_sides < 3) {
    DEBUGF("Can't clip primitive %p with %d sides\n",
           (void *)primitive, num_sides);
    return false;
  }

  int last_side = primitive_get_side(clipper, num_sides-1);
  bool last_inside = primitive_contains_point(
                       primitive, varray, last_side, plane);

  for (int t = 0; !(*split) && (t < num_sides); ++t) {
    const int side = primitive_get_side(clipper, t);
    DEBUGF("Front side %d: %d\n", t, side);

    const bool this_inside = primitive_contains_point(
                               primitive, varray, side, plane);
    if ((last_inside && this_inside) ||
        primitive_intersect(primitive, last_side, side, varray, plane)) {
      /* The back polygon contains or is intersected by this edge of the front
         primitive so we need to split it along the line of the edge. */
      if (!primitive_split(primitive, last_side, side,
                           varray, plane, out, split)) {
        DEBUGF("Clipping of primitive %p failed\n", (void *)primitive);
        return false;
      }
    }
    last_side = side;
    last_inside = this_inside;
  }
  DEBUGF("Clipping of primitive %p is complete\n", (void *)primitive);

  return true;
}

void primitive_set_used(const Primitive * const primitive,
                        const VertexArray * const varray)
{
  const int num_sides = primitive_get_num_sides(primitive);

  for (int s = 0; s < num_sides; ++s) {
    const int v = primitive_get_side(primitive, s);
    vertex_array_set_used(varray, v);
  }
}

int primitive_get_skew_side(Primitive * const primitive,
                            const VertexArray * const varray)
{
  const int num_sides = primitive_get_num_sides(primitive);

  if (num_sides < 4) {
    DEBUGF("Primitive %p with %d sides cannot be a skew polygon\n",
           (void *)primitive, num_sides);
  } else {
    /* Check for skew polygons. */
    const int v0 = primitive_get_side(primitive, 0);
    _Optional Coord (* const coords)[3] = vertex_array_get_coords(varray, v0);
    if (!coords) {
      return -1;
    }

    for (int s = 3; s < num_sides; ++s) {
       const int v = primitive_get_side(primitive, s);

      /* Check that each side of the primitive is orthogonal to the normal
         of the first two sides. The volume of the parallelepiped
         described by the first two sides of the primitive and each
         subsequent side must be 0. We compute that as the scalar triple
         product. */
      Coord (*side_old)[3] = vertex_array_get_coords(varray, v);
      if (!side_old) {
        return -1;
      }
      Coord side_new[3];
      vector_sub(&*side_old, &*coords, &side_new);

      if (primitive_ensure_normal(primitive, varray)) {
        const Coord volume = coord_abs(vector_dot(&primitive->normal, &side_new));

        if (!coord_equal(volume, 0)) {
          DEBUGF("Primitive %p is a skew polygon at side %d\n",
                 (void *)primitive, s);
          return s;
        }
      }
    }
    DEBUGF("Primitive %p is not a skew polygon\n", (void *)primitive);
  }

  return -1;
}

void primitive_print(const Primitive * const primitive,
                   const VertexArray * const varray)
{
  const int nsides = primitive_get_num_sides(primitive);
  for (int s = 0; s < nsides; ++s) {
    if (s > 0) {
      puts(",");
    }
    vertex_array_print_vertex(varray, primitive_get_side(primitive, s));
  }
}
