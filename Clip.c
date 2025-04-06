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
  CJB: 26-Aug-18: Removed the overlapping-bounding-boxes test, which is now
                  delegated to primitive_contains (which checks a stronger
                  constraint) and primitive_clip.
                  Failure of primitive_clip is now reported in verbose mode.
  CJB: 06-Apr-25: Dogfooding the _Optional qualifier.
 */

/* ISO library header files */
#include <stdbool.h>
#include <stdio.h>

/* Local header files */
#include "Vector.h"
#include "Coord.h"
#include "Vertex.h"
#include "Primitive.h"
#include "Group.h"
#include "Clip.h"
#include "Internal/3dObjMisc.h"

enum { MAX_SPLITS = 1024 };

static bool clip_group_vs_group(VertexArray * const varray,
                                Group * const groups,
                                const int bg, int const back,
                                const int fg, int front,
                                int *const nsplit, bool *const del,
                                const bool verbose)
{
  assert(groups != NULL);
  assert(fg >= 0);
  assert(bg >= 0);
  assert(front >= 0);
  assert(nsplit != NULL);
  assert(del != NULL);
  assert(!*del);

  Group * const back_group = groups + bg;
  DEBUGF("Back primitive is %d in group %d\n", back, bg);
  assert(back >= 0);
  _Optional Primitive *backp = group_get_primitive(back_group, back);
  if (!backp) {
    return false;
  }

  /* Find the two-dimensional plane in which to clip the two primitives
     (returns false if the back primitive is a point or line). */
  Plane plane;
  if (!primitive_find_plane(&*backp, varray, &plane)) {
    return true;
  }

  Group * const front_group = groups + fg;

  for (; front < group_get_num_primitives(front_group); ++front) {
    DEBUGF("Front primitive is %d in group %d\n", front, fg);
    _Optional Primitive *frontp = group_get_primitive(front_group, front);
    if (!frontp) {
      return false;
    }

    if (primitive_get_num_sides(&*frontp) < 3) {
      DEBUGF("Can't clip against point or line\n");
      continue;
    }

    if (!primitive_coplanar(&*frontp, &*backp, varray)) {
      continue;
    }

    bool split = false, covered = false;
    do {
      assert(!covered);
      if (primitive_equal(&*frontp, &*backp)) {
        covered = true;
        break;
      }

      if (primitive_contains(&*frontp, &*backp, varray, plane)) {
        /* The back polygon is completely covered by the front polygon */
        covered = true;
        break;
      }

      split = false;

      Primitive newbackp;
      if (!primitive_clip(&*backp, &*frontp, varray, plane, &newbackp, &split)) {
        if (verbose) {
          printf("Clipping failed (too many sides?)\n");
        }
        return false;
      }
      if (split) {
        assert(primitive_coplanar(&*frontp, &*backp, varray));
        assert(primitive_coplanar(&newbackp, &*backp, varray));
        _Optional Primitive * const r = group_insert_primitive(back_group, back + 1);
        if (r == NULL) {
          if (verbose) {
            printf("Clipping failed (out of memory)\n");
          }
          return false;
        }
        *r = newbackp;

        if (++(*nsplit) == MAX_SPLITS) {
          if (verbose) {
            printf("Aborted polygon clipping after %d splits\n", *nsplit);
          }
          return false;
        }

        /* A new polygon will have been inserted after the back polygon.
           If we are clipping against other primitives in the same group
           then that means the index of all following primitives (including
           the front polygon) increased by one. */
        if (front_group == back_group) {
          frontp = group_get_primitive(front_group, ++front);
          if (!frontp) {
            return false;
          }
        }

        /* In any case, the back group's primitive data may have moved. */
        backp = group_get_primitive(back_group, back);
        if (!backp) {
          return false;
        }

        if (verbose) {
          printf("Split polygon %d in group %d behind %d in group %d:\n",
                 primitive_get_id(&*backp), bg, primitive_get_id(&*frontp), fg);
          primitive_print(&*backp, varray);
          puts("\n and");
          _Optional Primitive *const behind = group_get_primitive(back_group, back + 1);
          if (!behind) {
            return false;
          }
          primitive_print(&*behind, varray);
          puts("");
        }
      } else {
        DEBUGF("No split\n");
      }
    } while (split);

    if (covered) {
      /* Report deletion of the back polygon here so that we know what
         caused it */
      if (verbose) {
        printf("Deleting polygon %d in group %d behind %d in group %d:\n",
               primitive_get_id(&*backp), bg, primitive_get_id(&*frontp), fg);
        primitive_print(&*backp, varray);
        puts("");
      }
      group_delete_primitive(back_group, back);
      *del = true;
      break; /* finish prematurely */
    }
  }

  return true;
}

static bool clip_group(VertexArray * const varray, Group * const groups,
                       const int *const group_order,
                       const int group_order_len, const int bg,
                       const bool verbose)
{
  /* Clip one group of polygons (selected according to the given group
     plot order) against any polygons in front of them. */
  assert(varray != NULL);
  assert(groups != NULL);
  assert(bg >= 0);
  assert(bg < group_order_len);

  int nsplit = 0, ndel = 0;

  DEBUGF("Back group is %d\n", group_order[bg]);
  Group * const back_group = groups + group_order[bg];

  /* Clip each polygon in the selected group in turn. */
  for (int back = 0; back < group_get_num_primitives(back_group); ++back) {
    bool del = false;

    /* Search for coplanar polygons in the same group as the
       polygon to be clipped. */
    if (!clip_group_vs_group(varray, groups, group_order[bg], back,
                             group_order[bg], back+1,
                             &nsplit, &del, verbose)) {
      return false;
    }

    /* Search for coplanar polygons in following groups
       (examined in the given group plot order). */
    for (int fg = bg + 1; !del && (fg < group_order_len); ++fg) {
      if (group_order[fg] == group_order[bg]) {
        DEBUGF("Cannot clip group %d against itself\n", group_order[fg]);
      } else {
        DEBUGF("Front group is %d\n", group_order[fg]);

        if (!clip_group_vs_group(varray, groups, group_order[bg], back,
                                 group_order[fg], 0,
                                 &nsplit, &del, verbose)) {
          return false;
        }
      }
    }

    if (del) {
      ++ndel;
      /* We've deleted the back polygon so we don't need to advance
         to the next polygon to be clipped on the next iteration. */
      --back;
    }
  }

  if (verbose) {
    if (nsplit || ndel) {
      printf("Split %d and deleted %d in group %d\n", nsplit, ndel, bg);
    }
  }

  return true;
}

bool clip_polygons(VertexArray * const varray,
                   Group * const groups,
                   const int *const group_order,
                   const int group_order_len,
                   const bool verbose)
{
  assert(varray != NULL);
  assert(groups != NULL);
  assert(group_order != NULL);
  assert(group_order_len >= 0);

  /* Clip each group of polygons in turn (using the given plot order). */
  for (int bg = 0; bg < group_order_len; ++bg) {
    if (!clip_group(varray, groups, group_order, group_order_len, bg,
                    verbose)) {
      return false;
    }
  }

  return true;
}
