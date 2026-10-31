#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit SALAA UEFI source claims without claiming to build or test firmware."""
from pathlib import Path
import os
import subprocess

root = Path(__file__).resolve().parents[1]
readme = (root / "README.md").read_text()
required = [
    "not a reproducible firmware build",
    "not compatible with HakiOS",
    "No clean-checkout EDK II firmware build",
    "UFS Block I/O",
    "Roadmap",
    "not an EDK II firmware build",
]
for phrase in required:
    if phrase.lower() not in readme.lower():
        raise SystemExit(f"README is missing required status text: {phrase}")

for path in [
    "source/Platforms/Realme/SalaaPkg/SalaaPkg.dsc",
    "source/Platforms/Realme/SalaaPkg/SalaaPkg.fdf",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/PlatformSec.c",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/PlatformSec.inf",
]:
    if not (root / path).is_file():
        raise SystemExit(f"Required source record is missing: {path}")

try:
    tracked = subprocess.check_output(
        ["git", "-C", str(root), "ls-files", "-z"], text=False
    ).decode().split("\0")
except subprocess.CalledProcessError as exc:
    raise SystemExit("This audit must run in a Git checkout") from exc
binary_suffixes = {".fd", ".efi", ".dll", ".map", ".pdb", ".img", ".bin"}
committed_artifacts = [
    p for p in tracked
    if p and (Path(p).suffix.lower() in binary_suffixes or p.startswith("build/Build/"))
]
if committed_artifacts:
    raise SystemExit("Unverified generated artifacts are tracked: " + ", ".join(committed_artifacts))

# Report known gaps. This is deliberately a source audit, not an EDK II build.
blockers = [
    "SalaaPkg/Include/DXE.inc",
    "SalaaPkg/Include/SalaaMuDefaults.dsc.inc",
    "Silicon/MediaTek/MT6785Pkg/Library/MemoryMapLib/MemoryMapLib.inf",
    "Silicon/MediaTek/MT6785Pkg/Library/SerialPortLib/SerialPortLib.inf",
    "Silicon/MediaTek/MT6785Pkg/Drivers/ShellBootDxe/ShellBootDxe.inf",
]
summary = [
    "## SALAA UEFI source audit",
    "",
    "**Result:** repository/status consistency checked. No EDK II build or firmware execution was performed.",
    "",
    "Known build/compatibility blockers:",
]
for item in blockers:
    state = "present" if (root / "source" / item).exists() else "not supplied by this repository"
    summary.append(f"- `{item}` — {state}")
summary += [
    "",
    "GOP, UFS Block I/O, memory reservations, PSCI and HakiOS boot remain unverified. A green audit is not firmware compatibility evidence.",
]
report = "\n".join(summary) + "\n"
summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
if summary_path:
    with open(summary_path, "a", encoding="utf-8") as stream:
        stream.write(report)
print(report)
