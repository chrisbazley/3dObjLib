"""Check a linked 65C02 image and run it with uCsim for CTest."""
import re
import subprocess
import sys
from pathlib import Path


def main():
    simulator, image_name = sys.argv[1:]
    image = Path(image_name)
    output = image.with_name(image.stem + "-output.log")
    simulator_log = image.with_name(image.stem + "-simulator.log")
    for log in (output, simulator_log):
        log.unlink(missing_ok=True)

    # Zero page, hardware stack, static data, heap, code and simulator
    # interface occupy separate parts of the 65C02 address space.
    areas = set()
    for line in image.with_suffix(".map").read_text().splitlines():
        match = re.match(
            r"^(ZP|OSEG|_DATA|DATA|BSS|_CODE|GSINIT|GSFINAL|CODE|RODATA|XINIT)"
            r"\s+([0-9A-F]+)\s+([0-9A-F]+)", line)
        if match:
            area, address, size = match.groups()
            areas.add(area)
            address, size = int(address, 16), int(size, 16)
            if not size:
                continue
            if area in ("ZP", "OSEG"):
                low, high = 1, 0x100
            elif area in ("_DATA", "DATA", "BSS"):
                low, high = 0x0200, 0x0800
            else:
                low, high = 0x2000, 0xfff0
            if address < low or address + size > high:
                raise SystemExit(f"{area} exceeds the test memory layout")
    if "CODE" not in areas or "GSINIT" not in areas:
        raise SystemExit("No code/startup regions found in linker map")

    command = [simulator, "-t", "65C02", "-q", "-c", "-",
               "-I", f"if=rom[0xfff0],out={output}", str(image)]
    with simulator_log.open("w") as log:
        result = subprocess.run(command, input="run\nquit\n", text=True,
                                stdout=log, stderr=subprocess.STDOUT, timeout=60)
    transcript = output.read_text() if output.exists() else ""
    print(transcript, end="")
    if result.returncode or "3DOBJ TEST PASS" not in transcript.splitlines():
        raise SystemExit(f"65C02 test failed; see {simulator_log}")


if __name__ == "__main__":
    main()
