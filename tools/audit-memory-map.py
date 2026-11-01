#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check SALAA's static descriptor ranges for invalid sizes and overlap."""
from pathlib import Path
import re

source = Path(__file__).resolve().parents[1] / "source/Platforms/Realme/SalaaPkg/Library/MemoryMapLib/MemoryMapLib.c"
text = source.read_text()
ranges = []

for match in re.finditer(
    r'SYSTEM_RAM\s*\(\s*"([^"]+)"\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*\)',
    text,
):
    name, start, end = match.groups()
    start, end = int(start, 16), int(end, 16)
    if end < start:
        raise SystemExit(f"{name}: end is below start")
    ranges.append((start, end + 1, name, "system RAM"))

for match in re.finditer(
    r'\{\s*"([^"]+)"\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*,\s*(?:AddMem|NoHob)',
    text,
):
    name, start, size = match.groups()
    start, size = int(start, 16), int(size, 16)
    if size <= 0 or start + size <= start:
        raise SystemExit(f"{name}: invalid region extent")
    ranges.append((start, start + size, name, "memory/device map"))

for match in re.finditer(
    r'MMIO_REGION\s*\(\s*"([^"]+)"\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*\)',
    text,
):
    name, start, size = match.groups()
    start, size = int(start, 16), int(size, 16)
    if size <= 0:
        raise SystemExit(f"{name}: invalid MMIO size")
    ranges.append((start, start + size, name, "MMIO"))

if not ranges:
    raise SystemExit("No memory descriptors were parsed")

payload = re.search(
    r'\{\s*"XNU Payload Reserved"\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*,\s*(0x[0-9A-Fa-f]+)ULL\s*,\s*AddMem',
    text,
)
if payload is None:
    raise SystemExit("The HakiOS XNUP handoff region is not reserved")
payload_start, payload_size = (int(value, 16) for value in payload.groups())
if (payload_start, payload_size) != (0xF0000000, 0x04000000):
    raise SystemExit("The XNUP reservation must match HakiOS's 0xF0000000/64 MiB contract")

ranges.sort()
for previous, current in zip(ranges, ranges[1:]):
    if previous[1] > current[0]:
        raise SystemExit(
            f"Overlapping descriptors: {previous[2]} ends at 0x{previous[1]:x}; "
            f"{current[2]} starts at 0x{current[0]:x}"
        )

print(f"Validated {len(ranges)} non-overlapping SALAA memory/MMIO descriptors")
for start, end, name, kind in ranges:
    print(f"{start:#x}-{end - 1:#x} {kind}: {name}")
