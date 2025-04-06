/*
 * 3dObjLib: Primitive groups
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
  CJB: 06-Apr-25: Dogfooding the _Optional qualifier.
 */

#ifndef GROUP_H
#define GROUP_H

#include "Primitive.h"
#include "Vertex.h"

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

typedef struct {
  int nalloc;
  int nprimitives;
  _Optional Primitive *primitives;
} Group;

void group_init(Group *group);

void group_delete_all(Group *group);

void group_free(Group *group);

int group_get_num_primitives(const Group *group);

_Optional Primitive *group_get_primitive(const Group *group, int n);

int group_alloc_primitives(Group *group, int n);

_Optional Primitive *group_add_primitive(Group *group);

_Optional Primitive *group_insert_primitive(Group *group, int prev);

void group_delete_primitive(Group *group, int n);

void group_set_used(const Group *group, VertexArray *varray);

#endif /* GROUP_H */
