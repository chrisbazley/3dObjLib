"""Check a linked SDCC test image and run it with uCsim for CTest."""
import re
import subprocess
import sys
from pathlib import Path


def main():
    simulator, target, image_name = sys.argv[1:]
    image = Path(image_name)
    output = image.with_name(image.stem + "-output.log")
    simulator_log = image.with_name(image.stem + "-simulator.log")
    for log in (output, simulator_log):
        log.unlink(missing_ok=True)

    if target == "z80":
        cpu, simif = "Z80", 0xdf00
        regions = {area: (0, 0xdf00) for area in
                   ("_CODE", "_HOME", "_GSINIT", "_GSFINAL", "_INITIALIZER")}
        regions.update({area: (0xf000, 0xf400) for area in ("_DATA", "_INITIALIZED")})
        required = {"_CODE"}
    else:
        cpu, simif = "65C02", 0xfff0
        regions = {area: (0x2000, 0xfff0) for area in
                   ("_CODE", "GSINIT", "GSFINAL", "CODE", "RODATA", "XINIT")}
        regions.update({area: (1, 0x100) for area in ("ZP", "OSEG")})
        regions.update({area: (0x0200, 0x0800) for area in ("_DATA", "DATA", "BSS")})
        required = {"CODE", "GSINIT"}

    areas = set()
    for line in image.with_suffix(".map").read_text().splitlines():
        match = re.match(r"^(\w+)\s+([0-9A-F]+)\s+([0-9A-F]+)", line)
        if match and match[1] in regions:
            area, address, size = match.groups()
            areas.add(area)
            address, size = int(address, 16), int(size, 16)
            low, high = regions[area]
            if size and (address < low or address + size > high):
                raise SystemExit(f"{area} exceeds the test memory layout")
    if not required <= areas:
        raise SystemExit("No code/startup regions found in linker map")

    command = [simulator, "-t", cpu, "-q", "-c", "-",
               "-I", f"if=rom[{simif:#x}],out={output}", str(image)]
    with simulator_log.open("w") as log:
        result = subprocess.run(command, input="run\nquit\n", text=True,
                                stdout=log, stderr=subprocess.STDOUT, timeout=60)
    transcript = output.read_text() if output.exists() else ""
    print(transcript, end="")
    if result.returncode or "3DOBJ TEST PASS" not in transcript.splitlines():
        raise SystemExit(f"{cpu} test failed; see {simulator_log}")


if __name__ == "__main__":
    main()
