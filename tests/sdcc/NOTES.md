# SDCC 65C02 investigation

Tested with the official SDCC 4.6.4 trunk r16999 Linux snapshot and its
`ucsim_mos6502 -t 65C02`. The CMake build compiles all five geometry modules
(Clip, Group, Primitive, Vector and Vertex), with `_Optional` enabled.
OBJ file output needs FILE streams absent from SDCC's library.

## Runtime coverage and limits

The default run covers 24 existing cases with active assertions:

| Program | Cases |
| --- | --- |
| Coord | 5 |
| Vector | 11 |
| Vertex1 (storage) | 7 |
| Vertex4 (printing) | 1 |

Vertex is split into smaller programs by VERTEX_TEST_PART. Its native suite
still runs all cases, including Fortify cases when enabled.
The Vector line-intersection test uses static arrays to reduce pressure on
the 256-byte hardware stack. The original automatic arrays failed the
`ex != 0` assertion in `Vector.c:236`; changing only their storage duration
made the suite pass. No assertion or test case was removed.

The other suites are excluded from the default simulator run:

- Vertex2 (finite edge intersection): fails the `ex != 0` assertion at
  `Vector.c:236` even when built as a program containing only that case.
- Vertex3 (edge against infinite line): its call to
  `vertex_array_edge_intersects_line` returns false, failing the assertion
  in `VertexTest.c`'s test9. This was previously hidden behind test8's
  earlier failure. It also fails when built as its own program. These
  failures need reduction before attributing them to compiler bugs; merely
  splitting the suite does not solve them.
- Primitive, Group and Clip: the initial linked CODE areas alone occupied
  79,933, 65,289 and 79,610 bytes respectively, exceeding the space available
  in a 64 KiB image once startup, data, heap and strings are included.

To reproduce the intersection failures, configure a separate build with
`-DSDCC_VERTEX_TEST_PARTS='2;3'`, then build and run CTest normally. The
assertions and case bodies are unchanged by the split.

To build an excluded complete suite, use e.g. `-DSDCC_TEST_GROUPS=Group`.
Splitting Group into two parts, Primitive into six, and Clip into five still
overflowed the available address space. `--opt-code-size` did not make those
split programs fit either, so those unsuccessful partitions are not kept
in the sources. The runner rejects map regions overlapping reserved memory,
including cases where the linker reports overflow but returns success.

The compiler's default calling convention exhausted zero page even with
`--model-large` for the Vector image (295 bytes in ZP plus 12 in OSEG).
`--stack-auto` avoids that limit and supports the qsort callback, but makes
hardware stack depth and image size important constraints. CBUtil is used
only for its headers here; its compiled modules are not needed by geometry.

## Library compatibility

SDCC supplies float maths, without the double fabs/sqrt interfaces, INFINITY
or isfinite. Coord therefore uses float, fabsf/sqrtf, an infinity expression,
and a finite-value wrapper in this build. The existing tolerance is already
documented as permitting single precision. The Coord suite checks infinity
as well as absolute value, square root, equality and ordering.

## Diagnostics to investigate

Warnings remain visible and nonfatal. Only SDCC's unreachable-code (126)
and integer-to-bool conversion (110) warnings are disabled, consistently
with CBUtil's existing build. No `_Optional` warning is disabled.

The clean default build produced 30 warning 196 diagnostics (pointer target
lost `_Optional`) and 3 warning 359 diagnostics (pointer arithmetic
not proved non-null), plus two warning 355 diagnostics (dereference not
proved non-null) in the newly enabled Vertex storage cases. The table records every location for later reduction;
it does not classify these as compiler bugs or source bugs. Many 196 sites
pass guarded pointers to coordinate arrays using `&*pointer`.

| Warning | File | Lines |
| --- | --- | --- |
| 196 | Primitive.c | 190, 409, 434, 538, 622, 626, 627, 653, 657, 658, 659, 660, 729, 769, 897, 905, 1160 |
| 196 | Vertex.c | 362, 436, 441, 442, 459, 460, 478, 509, 510, 534, 535, 555 |
| 196 | tests/VertexTest.c | 80 |
| 355 | tests/VertexTest.c | 73, 80 |
| 359 | Vertex.c | 300, 326, 390 |
| 94 | Vertex.c | 328 |

Warning 94 is the existing `ptrdiff_t <= INT_MAX` assertion, tautological
with this target's integer widths. There is also a preprocessor warning
because SDCC math.h and CBUtil MacroUtils.h both define PI with different
spellings. Neither warning has been suppressed or worked around here.
The workflow retains the complete build log and simulator logs as artifacts.
