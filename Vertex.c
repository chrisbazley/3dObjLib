/*
 * 3dObjLib: Vertex storage
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
  CJB: 26-Aug-18: Removed a spurious dependency on "ObjFile.h".
                  Optimised usage of vector_x and vector_y.
                  Modified vertex_array_edges_intersect and
                  vertex_array_edge_intersects_line to use coord_less_than
                  instead of custom expressions.
                  vertex_array_edges_intersect now reports intersections at
                  the start and end coordinates of edges.
  CJB: 28-Aug-18: vertex_array_find_duplicates now returns the number of
                  duplicates or a failure indication.
  CJB: 30-Aug-18: Added a function to mark all vertices as used.
  CJB: 09-Jan-21: Initialize struct using compound literal assignment to
                  guard against leaving members uninitialized.
 */

/* ISO library header files */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

/* Local header files */
#include "Internal/3dObjMisc.h"
#include "Vector.h"
#include "Coord.h"
#include "Vertex.h"

void vertex_array_init(VertexArray * const varray)
{
  assert(varray != NULL);
  *varray = (VertexArray){
    .nalloc = 0,
    .nvertices = 0,
    .nsorted = 0,
    .vertices = NULL,
    .sorted = NULL,
  };
}

void vertex_array_clear(VertexArray * const varray)
{
  varray->nvertices = 0;
}

void vertex_array_free(VertexArray * const varray)
{
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);
  free(varray->vertices);
  free(varray->sorted);
}

Vertex *vertex_array_get_vertex(const VertexArray * const varray, const int n)
{
  Vertex *vertex = NULL;

  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);

  if ((n >= 0) && (n < varray->nvertices)) {
    vertex = &varray->vertices[n];
  } else {
    DEBUGF("Invalid vertex number %d\n", n);
  }
  return vertex;
}

int vertex_array_get_num_vertices(const VertexArray * const varray)
{
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);
  return varray->nvertices;
}

void vertex_array_set_all_used(const VertexArray *varray)
{
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);

  const int nvertices = varray->nvertices;
  for (int v = 0; v < nvertices; ++v) {
    vertex_array_set_used(varray, v);
  }
}

void vertex_array_set_used(const VertexArray * const varray, const int n)
{
  Vertex * const vertex = vertex_array_get_vertex(varray, n);
  if (vertex != NULL) {
    DEBUGF("Marking vertex %d\n", n);
    vertex->marked = true;
  }
}

bool vertex_array_is_used(const VertexArray * const varray, const int n)
{
  bool is_used = false;
  Vertex * const vertex = vertex_array_get_vertex(varray, n);
  if (vertex != NULL) {
    is_used = vertex->marked;
    DEBUGF("Vertex %d is%s marked\n", n, is_used ? "" : " not");
  }
  return is_used;
}

int vertex_array_get_id(const VertexArray * const varray, const int n)
{
  int id = -1;
  Vertex *vertex = vertex_array_get_vertex(varray, n);
  while ((vertex != NULL) && (vertex->dup >= 0)) {
    DEBUGF("Vertex %d duplicates %d\n", n, vertex->dup);
    vertex = vertex_array_get_vertex(varray, vertex->dup);
  }
  if (vertex != NULL) {
    id = vertex->id;
  }
  DEBUGF("Vertex %d has ID %d\n", n, id);
  return id;
}

Coord (*vertex_array_get_coords(const VertexArray * const varray, const int n))[3]
{
  Coord (*coords)[3] = NULL;
  Vertex * const vertex = vertex_array_get_vertex(varray, n);
  if (vertex != NULL) {
    coords = &vertex->coords;
  }
  return coords;
}

int vertex_array_alloc_vertices(VertexArray * const varray, const int n)
{
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);

  if (n >= 0) {
    if (n > varray->nalloc) {
      int new_n = varray->nalloc ? varray->nalloc * 2 : 8;
      if (new_n < n) {
        new_n = n;
      }
      const size_t nbytes = sizeof(Vertex) * new_n;
      Vertex * const new_alloc = realloc(varray->vertices, nbytes);
      if (new_alloc == NULL) {
        DEBUGF("Failed to allocate %zu bytes for vertices\n", nbytes);
      } else {
        varray->vertices = new_alloc;
        varray->nalloc = new_n;
      }
    }
  } else {
    DEBUGF("Invalid number of vertices %d\n", n);
  }

  assert(varray->nvertices <= varray->nalloc);
  return varray->nalloc;
}

int vertex_array_add_vertex(VertexArray * const varray, Coord (* const coords)[3])
{
  int v = -1;

  assert(varray != NULL);
  assert(coords != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices < INT_MAX);

  const int new_nvert = varray->nvertices + 1;
  if (vertex_array_alloc_vertices(varray, new_nvert) >= new_nvert) {
    v = varray->nvertices++;
    assert(varray->nvertices <= varray->nalloc);

    Vertex * const vertex = vertex_array_get_vertex(varray, v);
    assert(vertex != NULL);

    *vertex = (Vertex){
      .marked = false,
      .id = v,
      .dup = -1,
    };

    for (size_t dim = 0; dim < ARRAY_SIZE(*coords); ++dim) {
      vertex->coords[dim] = (*coords)[dim];
    }

    DEBUGF("Added vertex %d {%"PCOORD",%"PCOORD",%"PCOORD"}\n", v,
           (*coords)[0], (*coords)[1], (*coords)[2]);
  }

  return v;
}

static int compare_vertices(const void *a, const void *b)
{
  /* We are sorting an array of pointers to vertices, so a and b are pointers
     to pointers. Dereference a and b to get plain pointers to vertices. */
  const Vertex * const v1 = *(Vertex **)a, * const v2 = *(Vertex **)b;
  assert(v1 != NULL);
  assert(v2 != NULL);

  if (v1 != v2) {
    for (size_t dim = 0; dim < ARRAY_SIZE(v1->coords); ++dim) {
      if (v1->coords[dim] < v2->coords[dim]) {
        DEBUGF("{%"PCOORD",%"PCOORD",%"PCOORD"} < "
               "{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
               v1->coords[0], v1->coords[1], v1->coords[2],
               v2->coords[0], v2->coords[1], v2->coords[2]);
        return -1;
      }
      if (v1->coords[dim] > v2->coords[dim]) {
        DEBUGF("{%"PCOORD",%"PCOORD",%"PCOORD"} > "
               "{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
               v1->coords[0], v1->coords[1], v1->coords[2],
               v2->coords[0], v2->coords[1], v2->coords[2]);
        return +1;
      }
    }
  }
  DEBUGF("{%"PCOORD",%"PCOORD",%"PCOORD"} == "
         "{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
         v1->coords[0], v1->coords[1], v1->coords[2],
         v2->coords[0], v2->coords[1], v2->coords[2]);
  return 0;
}

int vertex_array_find_duplicates(VertexArray * const varray,
                                 const bool verbose)
{
  int n = 0;
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);

  const int nvertices = varray->nvertices;
  if (nvertices > 0) {
    /* Allocate a temporary array of pointers to vertices */
    if (nvertices > varray->nsorted) {
      int new_n = varray->nsorted ? varray->nsorted * 2 : 8;
      if (new_n < nvertices) {
        new_n = nvertices;
      }
      const size_t nbytes = sizeof(Vertex *) * new_n;
      free(varray->sorted);
      varray->nsorted = 0;
      varray->sorted = malloc(nbytes);
      if (varray->sorted == NULL) {
        if (verbose) {
          printf("Failed to allocate %zu bytes for sorted vertices\n",
                 nbytes);
        }
        return -1;
      }
      varray->nsorted = nvertices;
    }
    Vertex ** const sorted = varray->sorted;

    for (int v = 0; v < nvertices; ++v) {
      sorted[v] = &varray->vertices[v];
    }

    /* Sort the array of pointers */
    qsort(sorted, nvertices, sizeof(Vertex *), compare_vertices);

    /* Check for duplicate neighbouring vertices in the sorted array.
       This should be done before marking vertices as used otherwise
       we may end up in a situation where a duplicate vertex is kept
       but the original is discarded */
    int last = 0;
    for (int v = 1 /* intentional */; v < nvertices; ++v) {
      assert(sorted[last] != sorted[v]);
      if (vector_equal(&sorted[last]->coords, &sorted[v]->coords)) {
        ++n;
        if (verbose) {
          printf("Vertex %d duplicates %d {%"PCOORD",%"PCOORD",%"PCOORD"}\n",
                  sorted[v]->id, sorted[last]->id,
                  sorted[v]->coords[0], sorted[v]->coords[1],
                  sorted[v]->coords[2]);
        }

        /* Link the duplicate vertex to the original so that querying its ID
           returns the original vertex's ID (whatever that turns out to be
           after renumbering all of the vertices). */
        assert(sorted[last] >= varray->vertices);
        sorted[v]->dup = sorted[last] - varray->vertices;

        /* To ensure that the original vertex is output, it must be marked if
           any of the vertices linked to it are marked. */
        if (sorted[v]->marked) {
          sorted[last]->marked = true;

          /* To avoid outputting duplicate vertices, they must be unmarked.
           */
          sorted[v]->marked = false;
        }
      } else {
        last = v;
      }
    }
  }
  if (verbose) {
    printf("%d/%d vertices were duplicates\n", n, varray->nvertices);
  }
  return n;
}


int vertex_array_find_vertex(const VertexArray * const varray, Coord (* const coords)[3])
{
  int found = -1;
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);

  const int nvertices = varray->nvertices;
  for (int v = 0; v < nvertices; ++v) {
    if (vector_equal(vertex_array_get_coords(varray, v), coords)) {
      found = v;
      break;
    }
  }

  if (found < 0) {
    DEBUGF("No vertex has coordinates {%"PCOORD",%"PCOORD",%"PCOORD"}\n",
           (*coords)[0], (*coords)[1], (*coords)[2]);
  } else {
    DEBUGF("Found coordinates {%"PCOORD",%"PCOORD",%"PCOORD"} as vertex %d\n",
           (*coords)[0], (*coords)[1], (*coords)[2], found);
  }

  return found;
}

int vertex_array_renumber(VertexArray * const varray, const bool verbose)
{
  assert(varray != NULL);
  assert(varray->nvertices >= 0);
  assert(varray->nvertices <= varray->nalloc);

  /* Renumber marked vertices */
  int next_id = 0;
  const int nvertices = varray->nvertices;
  for (int v = 0; v < nvertices; ++v) {
    Vertex * const vertex = vertex_array_get_vertex(varray, v);
    assert(vertex != NULL);
    if (vertex->marked) {
      /* Keep this vertex */
      if (next_id != v) {
        if (verbose) {
          printf("Renumbering vertex %d as %d "
                 "{%"PCOORD",%"PCOORD",%"PCOORD"}\n",
                 vertex->id, next_id,
                 vertex->coords[0], vertex->coords[1], vertex->coords[2]);
        }
        vertex->id = next_id;
      }
      ++next_id;
    }
  }
  if (verbose) {
    printf("%d/%d vertices survived\n", next_id, varray->nvertices);
  }
  return next_id;
}

/* This function treats the line CD as infinite in extent.
   with A inclusive start and B as exclusive end. */
bool vertex_array_edge_intersects_line(const VertexArray * const varray,
                                       const int a, const int b,
                                       const int c, const int d,
                                       const Plane p,
                                       Coord (* const intersect)[3])
{
  DEBUGF("Testing edge A(%d) .. B(%d) against line C(%d) .. D(%d)\n",
          a, b, c, d);
  assert(a != b);
  assert(c != d);
  assert(intersect != NULL);

  Coord (* const va)[3] = vertex_array_get_coords(varray, a);
  Coord (* const vb)[3] = vertex_array_get_coords(varray, b);
  Coord (* const vc)[3] = vertex_array_get_coords(varray, c);
  Coord (* const vd)[3] = vertex_array_get_coords(varray, d);

  if (!vector_intersect(va, vb, vc, vd, p, intersect)) {
    return false;
  }

  const Coord ix = *vector_x(intersect, p),
              ax = *vector_x(va, p),
              bx = *vector_x(vb, p);

  const Coord low_x = LOWEST(ax, bx);
  if (coord_less_than(ix, low_x)) {
    DEBUGF("Intersection at x=%"PCOORD" is left of edge %"PCOORD"\n",
           ix, low_x);
    return false;
  }

  const Coord high_x = HIGHEST(ax, bx);
  if (coord_less_than(high_x, ix)) {
    DEBUGF("Intersection at x=%"PCOORD" is right of edge %"PCOORD"\n",
           ix, high_x);
    return false;
  }

  const Coord iy = *vector_y(intersect, p),
              ay = *vector_y(va, p),
              by = *vector_y(vb, p);

  const Coord low_y = LOWEST(ay, by);
  if (coord_less_than(iy, low_y)) {
    DEBUGF("Intersection at y=%"PCOORD" is below edge %"PCOORD"\n",
           iy, low_y);
    return false;
  }

  const Coord high_y = HIGHEST(ay, by);
  if (coord_less_than(high_y, iy)) {
    DEBUGF("Intersection at y=%"PCOORD" is above edge %"PCOORD"\n",
           iy, high_y);
    return false;
  }

  /* Treat the endpoint as exclusive to avoid detecting the same
     intersection twice at each vertex of a primitive. */
  if (vector_equal(intersect, vb)) {
    DEBUGF("Ignoring intersection at B\n");
    return false;
  }

  return true;
}

/* This function treats AB and CD as edges of finite extent
   with inclusive starts and ends. */
bool vertex_array_edges_intersect(const VertexArray * const varray,
                                  const int a, const int b,
                                  const int c, const int d,
                                  const Plane p,
                                  Coord (* const intersect)[3])
{
  DEBUGF("Testing edge A(%d) .. B(%d) against edge C(%d) .. D(%d)\n",
          a, b, c, d);
  assert(a != b);
  assert(c != d);
  assert(intersect != NULL);

  Coord (* const va)[3] = vertex_array_get_coords(varray, a);
  Coord (* const vb)[3] = vertex_array_get_coords(varray, b);
  Coord (* const vc)[3] = vertex_array_get_coords(varray, c);
  Coord (* const vd)[3] = vertex_array_get_coords(varray, d);

  const Coord ax = *vector_x(va, p), bx = *vector_x(vb, p),
              cx = *vector_x(vc, p), dx = *vector_x(vd, p);

  const Coord ab_low_x = LOWEST(ax, bx);
  const Coord ab_high_x = HIGHEST(ax, bx);
  DEBUGF("AB %"PCOORD" <= x <= %"PCOORD")\n", ab_low_x, ab_high_x);

  const Coord cd_low_x = LOWEST(cx, dx);
  const Coord cd_high_x = HIGHEST(cx, dx);
  DEBUGF("CD %"PCOORD" <= x <= %"PCOORD")\n", cd_low_x, cd_high_x);

  /* These comparisons are meant to be less/greater than
     but we want the comparison to be inexact. */
  if (coord_less_than(cd_high_x, ab_low_x)) {
    DEBUGF("CD (%"PCOORD") is left of AB (%"PCOORD")\n",
           cd_high_x, ab_low_x);
    return false;
  }

  if (coord_less_than(ab_high_x, cd_low_x)) {
    DEBUGF("AB (%"PCOORD") is right of CD (%"PCOORD")\n",
           cd_low_x, ab_high_x);
    return false;
  }

  const Coord ay = *vector_y(va, p), by = *vector_y(vb, p),
              cy = *vector_y(vc, p), dy = *vector_y(vd, p);

  const Coord ab_low_y = LOWEST(ay, by);
  const Coord ab_high_y = HIGHEST(ay, by);
  DEBUGF("AB %"PCOORD" <= y <= %"PCOORD"\n", ab_low_y, ab_high_y);

  const Coord cd_low_y = LOWEST(cy, dy);
  const Coord cd_high_y = HIGHEST(cy, dy);
  DEBUGF("CD %"PCOORD" <= y <= %"PCOORD"\n", cd_low_y, cd_high_y);

  if (coord_less_than(cd_high_y, ab_low_y)) {
    DEBUGF("CD (%"PCOORD") is below AB (%"PCOORD")\n", cd_high_y, ab_low_y);
    return false;
  }

  if (coord_less_than(ab_high_y, cd_low_y)) {
    DEBUGF("CD (%"PCOORD") is above AB (%"PCOORD")\n", cd_low_y, ab_high_y);
    return false;
  }

  if (!vector_intersect(va, vb, vc, vd, p, intersect)) {
    return false;
  }

  const Coord ix = *vector_x(intersect, p);

  const Coord low_x = HIGHEST(ab_low_x, cd_low_x);
  if (coord_less_than(ix, low_x)) {
    DEBUGF("Intersection at x=%"PCOORD" is left of overlap %"PCOORD"\n",
           ix, low_x);
    return false;
  }

  const Coord high_x = LOWEST(ab_high_x, cd_high_x);
  if (coord_less_than(high_x, ix)) {
    DEBUGF("Intersection at x=%"PCOORD" is right of overlap %"PCOORD"\n",
           ix, high_x);
    return false;
  }

  const Coord iy = *vector_y(intersect, p);

  const Coord low_y = HIGHEST(ab_low_y, cd_low_y);
  if (coord_less_than(iy, low_y)) {
    DEBUGF("Intersection at y=%"PCOORD" is below overlap %"PCOORD"\n",
           iy, low_y);
    return false;
  }

  const Coord high_y = LOWEST(ab_high_y, cd_high_y);
  if (coord_less_than(high_y, iy)) {
    DEBUGF("Intersection at y=%"PCOORD" is above overlap %"PCOORD"\n",
           iy, high_y);
    return false;
  }

  return true;
}

void vertex_array_print_vertex(const VertexArray * const varray, const int v)
{
  Coord (* const coords)[3] = vertex_array_get_coords(varray, v);
  printf("%d:{%"PCOORD",%"PCOORD",%"PCOORD"}", v,
         (*coords)[0], (*coords)[1], (*coords)[2]);
}
