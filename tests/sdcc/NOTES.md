# SDCC 65C02 investigation

Tested with the official SDCC 4.6.4 trunk r16999 Linux snapshot and its
`ucsim_mos6502 -t 65C02`. The CMake build compiles all five geometry modules
(Clip, Group, Primitive, Vector and Vertex), with `_Optional` enabled.
OBJ file output needs FILE streams absent from SDCC's library.

## Runtime coverage and limits

Coord (five cases) and Vector (eleven cases) pass with active assertions.
The Vector line-intersection fixture uses static storage to reduce pressure
on the 256-byte hardware stack. The original automatic fixture failed the
`ex != 0` assertion in `Vector.c:236`; moving that fixture alone to static
storage made the suite pass. No assertion or test case was removed.

The other suites are excluded from the default simulator run:

- Vertex: the unchanged suite reaches finite edge intersection, then fails
  the `ex != 0` assertion at `Vector.c:236`. An exploratory change moving
  that test's VertexArray and intersection array to static storage instead
  reached the test's result assertion at `VertexTest.c:173`. That change is
  not included. Stack exhaustion is a concern, but the remaining failure
  needs reduction before attributing it to a compiler bug.
- Primitive, Group and Clip: the initial linked CODE areas alone occupied
  79,933, 65,289 and 79,610 bytes respectively, exceeding the space available
  in a 64 KiB image once startup, data, heap and strings are included.

To reproduce an excluded suite, configure a separate build with e.g.
`-DSDCC_TEST_GROUPS=Vertex`, then build and run CTest normally. Use
`-DSDCC_TEST_GROUPS='Primitive;Group;Clip'` for the oversized suites. The
runner rejects map regions that overlap reserved memory, including cases
where the linker prints an overflow diagnostic but returns success.

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

The initial clean build produced 29 warning 196 diagnostics (pointer target
lost `_Optional`) and three warning 359 diagnostics (pointer arithmetic
not proved non-null). The table records every location for later reduction;
it does not classify these as compiler bugs or source bugs. Many 196 sites
pass guarded pointers to coordinate arrays using `&*pointer`.

| Warning | File | Lines |
| --- | --- | --- |
| 196 | Primitive.c | 190, 409, 434, 538, 622, 626, 627, 653, 657, 658, 659, 660, 729, 769, 897, 905, 1160 |
| 196 | Vertex.c | 362, 436, 441, 442, 459, 460, 478, 509, 510, 534, 535, 555 |
| 359 | Vertex.c | 300, 326, 390 |
| 94 | Vertex.c | 328 |

Warning 94 is the existing `ptrdiff_t <= INT_MAX` assertion, tautological
with this target's integer widths. There is also a preprocessor warning
because SDCC math.h and CBUtil MacroUtils.h both define PI with different
spellings. Neither warning has been suppressed or worked around here.
The workflow retains the complete build log and simulator logs as artifacts.
