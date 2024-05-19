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
  CJB: 09-Jan-21: Initialize struct using compound literal assignment to
                  guard against leaving members uninitialized.
*/

/* ISO library header files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Local header files */
#include "Internal/3dObjMisc.h"
#include "Primitive.h"
#include "Group.h"

void group_init(Group * const group)
{
  assert(group != NULL);
  *group = (Group){
    .nprimitives = 0,
    .nalloc = 0,
    .primitives = NULL,
  };
}

int group_get_num_primitives(const Group * const group)
{
  assert(group != NULL);
  assert(group->nprimitives >= 0);
  assert(group->nprimitives <= group->nalloc);
  return group->nprimitives;
}

void group_delete_all(Group * const group)
{
  assert(group != NULL);
  assert(group->nprimitives >= 0);
  assert(group->nprimitives <= group->nalloc);
  group->nprimitives = 0;
}

void group_free(Group * const group)
{
  assert(group != NULL);
  assert(group->nprimitives >= 0);
  assert(group->nprimitives <= group->nalloc);
  free(group->primitives);
}

Primitive *group_get_primitive(const Group * const group, const int n)
{
  Primitive *primitive = NULL;

  assert(group != NULL);
  assert(group->nprimitives >= 0);
  assert(group->nprimitives <= group->nalloc);

  if ((n >= 0) && (n < group->nprimitives)) {
    primitive = group->primitives + n;
  } else {
    DEBUGF("Invalid primitive number %d\n", n);
  }

  return primitive;
}

Primitive *group_add_primitive(Group * const group)
{
  return group_insert_primitive(group, group->nprimitives);
}

int group_alloc_primitives(Group * const group, const int n)
{
  assert(group != NULL);
  assert(group->nprimitives >= 0);
  assert(group->nprimitives <= group->nalloc);

  if (n >= 0) {
    if (n > group->nalloc) {
      const int new_nalloc = group->nalloc ? group->nalloc * 2 : 8;
      const size_t nbytes = sizeof(Primitive) * new_nalloc;
      Primitive * const new_primitives = realloc(group->primitives, nbytes);
      if (new_primitives == NULL) {
        DEBUGF("Failed to allocate %zu bytes for primitives\n", nbytes);
      } else {
        DEBUGF("Moving primitives from %p to %p\n",
                (void *)group->primitives, (void *)new_primitives);
        group->primitives = new_primitives;
        group->nalloc = new_nalloc;
      }
    }
  } else {
    DEBUGF("Invalid number of primitives %d\n", n);
  }

  assert(group->nprimitives <= group->nalloc);
  return group->nalloc;
}

Primitive *group_insert_primitive(Group * const group, const int n)
{
  Primitive * primitive = NULL;

  assert(group != NULL);
  assert(group->nprimitives >= 0);
  assert(group->nprimitives <= group->nalloc);

  /* Allow insert at place beyond last */
  if ((n >= 0) && (n < group->nprimitives+1)) {
    const int new_nprim = group->nprimitives + 1;
    if (group_alloc_primitives(group, new_nprim) >= new_nprim) {
      ++group->nprimitives;
      assert(group->nprimitives <= group->nalloc);
      primitive = group_get_primitive(group, n);
      assert(primitive != NULL);

      if (group->nprimitives > (n+1)) {
         memmove(primitive + 1, primitive,
                 sizeof(Primitive) * (group->nprimitives - (n+1)));
      }

      primitive_init(primitive);

      DEBUGF("Added primitive %d (%p) in group %p\n", n,
              (void *)primitive, (void *)group);
    }
  } else {
     DEBUGF("Invalid primitive number %d\n", n);
  }

  return primitive;
}

void group_delete_primitive(Group * const group, const int n)
{
  Primitive *const primitive = group_get_primitive(group, n);
  if (primitive != NULL) {
    if (group->nprimitives > (n+1)) {
      memmove(primitive, primitive + 1,
              sizeof(Primitive) * (group->nprimitives - (n+1)));
    }
    --group->nprimitives;

    DEBUGF("Deleted primitive %d (%p) in group %p\n", n,
            (void *)primitive, (void *)group);
  }
}

void group_set_used(const Group * const group, VertexArray * const varray)
{
  const int nprimitives = group_get_num_primitives(group);

  for (int p = 0; p < nprimitives; ++p) {
    Primitive * const pp = group_get_primitive(group, p);
    assert(pp != NULL);
    primitive_set_used(pp, varray);
  }
}
