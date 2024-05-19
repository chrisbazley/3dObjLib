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
  CJB: 28-Aug-18: vertex_array_find_duplicates now returns the number of
                  duplicates or a failure indication.
  CJB: 30-Aug-18: Added a function to mark all vertices as used.
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
 */

#ifndef VERTEX_H
#define VERTEX_H

#include <stdint.h>
#include <stdbool.h>

#include "Vector.h"
#include "Coord.h"

typedef struct {
  Coord coords[3];
  int id;
  int dup;
  bool marked;
} Vertex;

typedef struct {
  int nalloc;
  int nvertices;
  int nsorted;
  Vertex *vertices;
  Vertex **sorted;
} VertexArray;

void vertex_array_init(VertexArray *varray);

void vertex_array_clear(VertexArray *varray);

void vertex_array_free(VertexArray *varray);

Vertex *vertex_array_get_vertex(const VertexArray *varray, int n);

int vertex_array_get_num_vertices(const VertexArray *varray);

void vertex_array_set_all_used(const VertexArray *varray);

void vertex_array_set_used(const VertexArray *varray, int n);

bool vertex_array_is_used(const VertexArray *varray, int n);

int vertex_array_get_id(const VertexArray *varray, int n);

Coord (*vertex_array_get_coords(const VertexArray *varray, int n))[3];

int vertex_array_alloc_vertices(VertexArray *varray, int n);

int vertex_array_add_vertex(VertexArray *varray, Coord (*coords)[3]);

int vertex_array_find_vertex(const VertexArray *varray, Coord (*coords)[3]);

int vertex_array_find_duplicates(VertexArray *varray, bool verbose);

int vertex_array_renumber(VertexArray *varray, bool verbose);

bool vertex_array_edge_intersects_line(const VertexArray *varray,
                                       int a, int b, int c, int d,
                                       Plane p, Coord (*intersect)[3]);

bool vertex_array_edges_intersect(const VertexArray *varray, int a, int b,
                                  int c, int d, Plane p,
                                  Coord (*intersect)[3]);

void vertex_array_print_vertex(const VertexArray *varray, int v);

#endif /* VERTEX_H */
