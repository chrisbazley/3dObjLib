/*
 * 3dObjLib: OBJ file generation
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
  CJB: 11-Dec-20: Removed redundant uses of the 'extern' keyword.
 */

#ifndef OBJFILE_H
#define OBJFILE_H

#include <stdio.h>
#include <stdbool.h>

#include "Primitive.h"
#include "Vertex.h"
#include "Group.h"

typedef enum {
  VertexStyle_Positive,
  VertexStyle_Negative
} VertexStyle;

typedef enum {
  MeshStyle_NoChange,
  MeshStyle_TriangleFan,
  MeshStyle_TriangleStrip
} MeshStyle;

bool output_vertices(
        FILE *out, int vobject, const VertexArray *varray,
        int rot);

bool output_primitives(
        FILE *out, const char *object_name,
        int vtotal, int vobject, const VertexArray *varray,
        Group const *groups, int ngroups,
        int (*get_colour)(const Primitive *pp, void *arg),
        int (*get_material)(char *buf, size_t buf_size,
                            int colour, void *arg),
        void *arg, VertexStyle vstyle, MeshStyle mstyle);

#endif /* OBJFILE_H */
