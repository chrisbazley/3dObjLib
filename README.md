# 3dObjLib
(C) 2018 Christopher Bazley

Release 12 (13 Apr 2025)

Preamble
--------
  This C library is designed for use by programs that process 3D models in
the simple object geometry format developed by Wavefront for their Advanced
Visualizer software. It should have minimal dependencies on other libraries
and must not include any platform-specific code.

Main features:
- Storage of geometric primitives, groups of primitives and vertices.
- 3D vector mathematics functions.
- Clipping of overlapping coplanar polygons (to avoid "Z-fighting").
- OBJ file generation.

Clipping
--------
  Some objects are liable to suffer from a phenomenon known as "Z-fighting"
if they are part of a scene rendered using a depth (Z) buffer. It is caused
by overlapping faces in the same geometric plane and typically manifests as
a shimmering pattern in a rendered image. Essentially two or more polygons
occupy the same points in 3D space.

  The clip_polygons function searches for overlapping coplanar polygons
within a specified set of groups of primitives. The caller must specify a
rendering order for the groups. Within each group, primitives are assumed to
be rendered in the order they were added to the group.

  The rearmost of two overlapping polygons is split by dividing it along the
line obtained by extending one edge of the front polygon to infinity in both
directions. This process is repeated until no two edges intersect. Any
polygons that are fully hidden (typically behind decals) are deleted.

  The following diagrams illustrate how one polygon (B: 1 2 3 4) overlapped
by another (A: 5 6 7 8) is split into five polygons (B..F) during the
clipping process. The last polygon (F) is then deleted because it duplicates
the overlapping polygon (A).
```
     Original         First split       Second split
  (A overlaps B)     (A overlaps C)    (A overlaps D)
                               :
  3_____________2   3__________9__2   3__________9__2
  |      B      |   |          |  |   |      C   |  |
  |  7_______6  |   |          |  | ..|__________6..|..
  |  |   A   |  |   |      C   |B | 11|          |B |
  |  |_______|  |   |          |  |   |      D   |  |
  |  8       5  |   |          |  |   |          |  |
  |_____________|   |__________|__|   |__________|__|
  4             1   4          10 1   4          10 1
                               :
    Third split       Fourth split     Final clipped
  (A overlaps E)     (A overlaps F)     (F deleted)
  3__:_______9__2   3__________9__2   3__________9__2
  |  :   C   |  |   |      C   |  |   |      C   |  |
11|__7_______6  | 11|__7_______6  | 11|__7_______6  |
  |  |       |B |   |D |   F   |B |   |D |   A   |B |
  |D |   E   |  | ..|..|_______|..|.. |  |_______|  |
  |  |       |  |   |  8   E   5  |   |  8   E   5  |
  |__|_______|__|   |__|_______|__|   |__|_______|__|
  4  12      10 1   4  12      10 1   4  12      10 1
     :
```
Output of faces
---------------
  The Wavefront OBJ format specification does not restrict the maximum
number of vertices in a face element. Nevertheless, some programs cannot
correctly display faces with more than three vertices. The output_primitives
function can optionally split complex polygons into triangles.
```
       Original          Triangle fans       Triangle strips
     3_________2          3_________2          3_________2
     /         \          / `-._    \          /\`-._    \
    /           \        /      `-._ \        /  `\  `-._ \
 4 /             \ 1  4 /___________`-\ 1  4 /     \.    `-\ 1
   \             /      \         _.-`/      \`-._   \     /
    \           /        \    _.-`   /        \   `-. \.  /
     \_________/          \.-`______/          \_____`-.\/
     5         6          5         6          5         6

 f 1 2 3 4 5 6        f 1 2 3              f 1 2 3
                      f 1 3 4              f 6 1 3
                      f 1 4 5              f 6 3 4
                      f 1 5 6              f 5 6 4
```
  Vertices in a face element are normally indexed by their position in the
output file, counting upwards from 1. If the output comprises more than one
object definition then it can be more useful to count backwards from the
most recent vertex definition, which is assigned index -1. The
output_primitives function can optionally use this output mode, which allows
object models to be separated, extracted or rearranged later.

Fortified memory allocation
---------------------------
  I use Simon's P. Bullen's fortified memory allocation shell 'Fortify' to
find memory leaks in my applications, detect corruption of the heap
(e.g. caused by writing beyond the end of a heap block), and do stress
testing (by causing some memory allocations to fail). Fortify is available
separately from this web site:
http://web.archive.org/web/20020615230941/www.geocities.com/SiliconValley/Horizon/8596/fortify.html

  By default, Fortify only intercepts the ANSI standard memory allocation
functions (e.g. 'malloc', 'free' and 'realloc'). This limits its usefulness
as a debugging tool if your program also uses different memory allocator such
as Acorn's 'flex' library.

  The debugging version of 3dObjLib must be linked with 'Fortify', for
example by adding 'C:o.Fortify' to the list of object files specified to the
linker. Otherwise, you will get build-time errors.

Rebuilding the library
----------------------
  You should ensure that the standard C library and CBDebugLib (by the same
author as 3dObjLib) are on your header include path (C$Path if using the
supplied make files on RISC OS), otherwise the compiler won't be able to find
the required header files. The dependency on CBDebugLib isn't very strong: it
can be eliminated by modifying the make file so that the macro USE_CBDEBUG is
no longer predefined.

  Three make files are supplied:

- 'Makefile' is intended for use with GNU Make and the GNU C Compiler on Linux.
- 'NMakefile' is intended for use with Acorn Make Utility (AMU) and the
   Norcroft C compiler supplied with the Acorn C/C++ Development Suite.
- 'GMakefile' is intended for use with GNU Make and the GNU C Compiler on RISC OS.

These make files share some variable definitions (including a list of
objects to be built) by including a common make file.

  The APCS variant specified for the Norcroft compiler is 32 bit for
compatibility with ARMv5 and fpe2 for compatibility with older versions of
the floating point emulator. Generation of unaligned data loads/stores is
disabled for compatibility with ARM v6.

  The suffix rules generate output files with different suffixes (or in
different subdirectories, if using the supplied make files on RISC OS),
depending on the compiler options used to compile the source code:

o: Assertions and debugging output are disabled. The code is optimised for
   execution speed.

debug: Assertions and debugging output are enabled. The code includes
       symbolic debugging data (e.g. for use with DDT). The macro FORTIFY
       is pre-defined to enable Simon P. Bullen's fortified shell for memory
       allocations.

d: 'GMakefile' passes '-MMD' when invoking gcc so that dynamic dependencies
   are generated from the #include commands in each source file and output
   to a temporary file in the directory named 'd'. GNU Make cannot
   understand rules that contain RISC OS paths such as /C:Macros.h as
   prerequisites, so 'sed', a stream editor, is used to strip those rules
   when copying the temporary file to the final dependencies file.

  The above suffixes must be specified in various system variables which
control filename suffix translation on RISC OS, including at least
UnixEnv$ar$sfix, UnixEnv$gcc$sfix and UnixEnv$make$sfix.
Unfortunately GNU Make doesn't apply suffix rules to make object files in
subdirectories referenced by path even if the directory name is in
UnixEnv$make$sfix, which is why 'GMakefile' uses the built-in function
addsuffix instead of addprefix to construct lists of the objects to be
built (e.g. foo.o instead of o.foo).

  Before compiling the library for RISC OS, move the C source and header
files with .c and .h suffixes into subdirectories named 'c' and 'h' and
remove those suffixes from their names. You probably also need to create
'o', 'd' and 'debug' subdirectories for compiler output.

Licence and disclaimer
----------------------
  This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

  This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
for more details.

  You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Credits
-------
  My information on the Wavefront Object file format came from Paul Bourke's
copy of the file format specification for the Advanced Visualizer software
(Copyright 1995 Alias|Wavefront, Inc.):
  http://paulbourke.net/dataformats/obj/

History
-------
Release 1 (19th August 2018)
- First stand-alone release under the L.G.P.L.

Release 2 (28th August 2018)
- Fixed a bug in the clipping algorithm: primitive_intersect must not ignore
  intersections between the given edge and a polygon which are coincident
  with the polygon's corners. Consequently vertex_array_edges_intersect now
  reports intersections at the start and end coordinates of both edges.
- primitive_delete_all no longer omits to reset the flags concerning any
  cached normal vertex and bounding box.
- Moved overlapping-bounding-boxes checks from the clipping code to
  primitive_contains (which checks a stronger constraint) and primitive_clip.
- Failure of primitive_clip is now reported in verbose mode.
- Optimised primitive_contains_point and primitive_get_top_y by using the
  cached bounding box.
- primitive_contains_point now returns early if the input vertex is part of
  the definition of the polygon.
- primitive_contains now returns early if any vertex of one polygon is found
  to be outside the other.
- primitive_equal and primitive_intersect now safely handle the case where a
  primitive has no sides.
- Created internal variants of primitive_get_normal and primitive_get_bbox
  for use when the result doesn't need to be copied.
- Added a macro to allow bounding box optimisations to be disabled (mainly
  for debugging).
- Added a coord_less_than function for use by primitive_contains_point,
  vertex_array_edges_intersect and vertex_array_edge_intersects_line.
- Added a vector_xy_greater_or_equal function for use in primitive bounding
  box checks.

Release 3 (28th August 2018)
- vertex_array_find_duplicates now returns the number of duplicates or a
  failure indication.

Release 4 (30th August 2018)
- Added a function to mark all vertices as used.

Release 5 (4th November 2018)
- Simplified primitive_get_top_y to work around a bug in the Norcroft C
  compiler (which previously crashed when compiling this function).
- Instead of all builds of the library depending on CBLib, now only the
  debug build has a dependency, and that is only on CBDebugLib.

Release 6 (17th November 2018)
- vector_x/y/z are no longer inline functions. This allows the header file
  in which they are declared to be included without defining macros.
- Plane indices are now bytes instead of size_t values. This seems to result
  in better object code being being generated for vector_x/y/z.

Release 7 (21 Apr 2020)
- Fixed bad output in the form of "# 1 vertice".

Release 8 (30 Jul 2022)
- Initialize structs using compound literal assignment to guard against
  leaving members uninitialized.
- Removed redundant uses of the 'extern' keyword.
- Extra range check in primitive_get_side() to stop a wrong warning from
  GCC's -Warray-bounds.

Release 9 (19 May 2024)
- Added a new make file for use on Linux.

Release 10 (06 Apr 2025)
- This version of the library has been used for dogfooding the _Optional
  qualifier (https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3422.pdf).

Release 11 (11 Apr 2025)
- Allow null argument to get_colour and get_material.

Release 12 (13 Apr 2025)
- Rename output_primitives_get_(colour|material) types.

Contact details
---------------
Christopher Bazley

Email: mailto:cs99cjb@gmail.com

WWW:   http://starfighter.acornarcade.com/mysite/
