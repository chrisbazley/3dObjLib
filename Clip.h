/*
 * 3dObjLib: Clip overlapping polygons
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

#ifndef CLIP_H
#define CLIP_H

/* ISO library header files */
#include <stdbool.h>

/* Local header files */
#include "Vertex.h"
#include "Group.h"

bool clip_polygons(VertexArray *varray, Group *groups,
                   const int *group_order, int group_order_len,
                   bool verbose);

#endif /* CLIP_H */
