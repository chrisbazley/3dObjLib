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
  CJB: 21-Apr-20: Fixed bad output in the form of "# 1 vertice".
  CJB: 06-Apr-25: Dogfooding the _Optional qualifier.
 */

/* ISO library header files */
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

/* Local header files */
#include "ObjFile.h"
#include "Coord.h"
#include "Vertex.h"
#include "Primitive.h"
#include "Internal/3dObjMisc.h"

static int convert_vnum(const VertexArray * const varray, int v,
                        const int vtotal, const int vobject,
                        const VertexStyle vstyle)
{
  assert(varray != NULL);
  assert(vtotal >= 0);
  assert(vobject > 0);

  const int id = vertex_array_get_id(varray, v);
  if (vstyle == VertexStyle_Negative) {
    assert(id <= vobject);
    v = - (vobject - id);
  } else {
    assert(vstyle == VertexStyle_Positive);
    v = 1 + vtotal + id;
  }
  return v;
}

bool output_vertices(FILE * const out, const int vobject,
                     const VertexArray * const varray,
                     const int rot)
{
  assert(out != NULL);
  assert(!ferror(out));
  assert(vobject > 0);

  if (fprintf(out, "\n# %d vertices\n", vobject) < 0) {
    return false;
  }

  const int nvertices = vertex_array_get_num_vertices(varray);
  for (int v = 0; v < nvertices; ++v) {
    if ((v == rot) && fputs("# Following vertices rotate\n", out) == EOF) {
      return false;
    }

    _Optional Coord (* const coords)[3] = vertex_array_get_coords(varray, v);
    if (!coords) {
      continue;
    }

    if (!vertex_array_is_used(varray, v)) {
      DEBUGF("Omitting vertex %d {%"PCOORD",%"PCOORD",%"PCOORD"} "
             "from the output\n",
             v, (*coords)[0], (*coords)[1], (*coords)[2]);
      continue;
    }

    if (fputs("v", out) == EOF) {
      return false;
    }
    for (size_t dim = 0; dim < ARRAY_SIZE(*coords); ++dim) {
      if (fprintf(out, " %f", (*coords)[dim]) < 0) {
        return false;
      }
    }
    if (fputs("\n", out) == EOF) {
      return false;
    }
  }

  return true;
}

static bool output_primitive(FILE * const out, const Primitive * const pp,
                           const int vtotal, const int vobject,
                           const VertexArray * const varray,
                           const VertexStyle vstyle,
                           const MeshStyle mstyle)
{
  assert(out != NULL);
  assert(!ferror(out));
  assert(vtotal >= 0);
  assert(vobject > 0);

  const int nsides = primitive_get_num_sides(pp);
  if ((nsides > 3) && (mstyle != MeshStyle_NoChange)) {
    int s, v[3] = {0, 0, 0};
    for (s = 0; s < 2; ++s) {
      v[s] = convert_vnum(
               varray, primitive_get_side(pp, s), vtotal, vobject, vstyle);
    }

    for (; s < nsides; ++s) {
      int sindex;
      if (mstyle == MeshStyle_TriangleFan) {
        /* Count up from index 2...N-1, where N is the no. of sides */
        sindex = s;
      } else {
        assert(mstyle == MeshStyle_TriangleStrip);
        if (s % 2) {
          /* Odd-numbered iterations count down from index N-1, N-2 .. */
          sindex = primitive_get_num_sides(pp) - ((s-1)/2);
        } else {
          /* Even-numbered iterations count up from index 2,3,4 .. */
          sindex = 1 + (s/2);
        }
      }

      /* Replace the first or third vertex (always replace the third
         when making triangle fans) */
      const int vnext = convert_vnum(
                          varray, primitive_get_side(pp, sindex), vtotal,
                          vobject, vstyle);
      if ((mstyle == MeshStyle_TriangleFan) || !(s % 2)) {
        v[2] = vnext;
      } else {
        assert(mstyle == MeshStyle_TriangleStrip);
        v[0] = vnext;
      }

      if (fputs("f", out) == EOF) {
        return false;
      }
      for (size_t ts = 0; ts < ARRAY_SIZE(v); ++ts) {
        if (fprintf(out, " %d", v[ts]) < 0) {
          return false;
        }
      }
      if (fputs("\n", out) == EOF) {
        return false;
      }

      /* Keep the first or third vertex for the next iteration
         (always keep the third when making triangle fans) */
      v[1] = ((mstyle == MeshStyle_TriangleFan) || (s % 2)) ? v[2] : v[0];
    } /* next side */
  } else {
    assert(nsides > 0);
    const char *primitive_type;
    switch (nsides) {
    case 1:
      primitive_type = "p";
      break;
    case 2:
      primitive_type = "l";
      break;
    default:
      primitive_type = "f";
      break;
    }
    if (fputs(primitive_type, out) == EOF) {
      return false;
    }
    for (int s = 0; s < nsides; ++s) {
      const int v = convert_vnum(
                      varray, primitive_get_side(pp, s),
                      vtotal, vobject, vstyle);
      if (fprintf(out, " %d", v) < 0) {
        return false;
      }
    } /* next side */
    if (fputs("\n", out) == EOF) {
      return false;
    }
  }

  return true;
}

bool output_primitives(FILE * const out, const char * const object_name,
                     const int vtotal, const int vobject,
                     const VertexArray * const varray,
                     Group const * const groups,
                     int const ngroups,
                     _Optional output_primitives_get_colour *get_colour,
                     _Optional output_primitives_get_material *get_material,
                     void *arg, const VertexStyle vstyle,
                     const MeshStyle mstyle)
{
  assert(out != NULL);
  assert(!ferror(out));
  assert(groups != NULL);
  assert(vtotal >= 0);
  assert(vobject > 0);

  int last_colour = INT_MAX;
  for (int g = 0; g < ngroups; ++g) {
    const Group *const group = groups + g;
    const int nprimitives = group_get_num_primitives(group);

    if (nprimitives > 0) {
      if (fprintf(out, "\n# %d primitives\n", nprimitives) < 0) {
        return false;
      }

      if (fprintf(out, "g %s %s_%d\n", object_name, object_name, g) < 0) {
        return false;
      }
    }

    for (int p = 0; p < nprimitives; ++p) {
      const _Optional Primitive * const pp = group_get_primitive(group, p);
      if (!pp) {
        return false;
      }
      int const colour = get_colour ?
                         get_colour(&*pp, arg) :
                         primitive_get_colour(&*pp);

      if (last_colour != colour) {
        char material[64];
        int n = get_material ?
                get_material(material, sizeof(material), colour, arg) :
                snprintf(material, sizeof(material), "colour_%d", colour);

        if (n < 0) {
          return false;
        }
        n = fprintf(out, "usemtl %s\n", material);
        if (n < 0) {
          return false;
        }
        last_colour = colour;
      }

      if (!output_primitive(out, &*pp, vtotal, vobject, varray, vstyle, mstyle)) {
        return false;
      }
    } /* next primitive */
  } /* next group */

  return true;
}
