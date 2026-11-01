#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit SALAA UEFI source claims without claiming to build or test firmware."""
from pathlib import Path
import hashlib
import os
import re
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

evidence_path = root / "docs/salaa-device-evidence.md"
if not evidence_path.is_file():
    raise SystemExit("The sanitized SALAA device-evidence report is missing")
evidence = evidence_path.read_text()
for phrase in (
    "0974ca677db2aaf3010b602d37cc123f33860a1d75e9f1fed735178f72e739ec",
    "0x200000000",
    "0x11270000",
    "0x11fa0000",
    "does **not** contain the original Mu-Silicium workspace",
):
    if phrase.lower() not in evidence.lower():
        raise SystemExit(f"SALAA evidence report is missing expected input fact: {phrase}")
if re.search(r"androidboot\.serialno=[^\s\"']+|serialno\s*=\s*\"[^\"]+\"", evidence, re.I):
    raise SystemExit("A device serial value must not be committed in the evidence report")

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
allowed_artifact_prefixes = (
    "artifacts/release-v0.1.0/",
    "artifacts/actions/",
)
committed_artifacts = [
    p for p in tracked
    if p and (Path(p).suffix.lower() in binary_suffixes or p.startswith("build/Build/"))
    and not p.startswith(allowed_artifact_prefixes)
]
if committed_artifacts:
    raise SystemExit("Unverified generated artifacts are tracked: " + ", ".join(committed_artifacts))

release_outputs = {
    "SALAA_EFI.release-v0.1.0.fd": (4_194_304, "5e6ab9e0e9376b634e5393b39b9889c671e746a49f3f582c90b858b75bf1ace7"),
    "SalaaSec.release-v0.1.0.efi": (38_912, "4e68dd708969cc6cb00a4ec375c1950d181a573778c8cd87f9c240fe4fbe9bbb"),
    "SalaaSec-PlatformSec.release-v0.1.0.efi": (38_912, "4e68dd708969cc6cb00a4ec375c1950d181a573778c8cd87f9c240fe4fbe9bbb"),
    "SalaaSec-PlatformSec.release-v0.1.0.dll": (516_504, "f8e5d6b22157f0e9d6c75ffde2b344ab275d77c13506afd00d7e3f545e9b277d"),
    "SalaaSec-PlatformSec.release-v0.1.0.map": (103_876, "8c105969769ace628df207c4051c50892dee0206ecd6b490824cbfc29cded642"),
}
for name, (expected_size, expected_hash) in release_outputs.items():
    path = root / "artifacts/release-v0.1.0" / name
    if not path.is_file() or path.stat().st_size != expected_size:
        raise SystemExit(f"Preserved v0.1.0 output is missing or has the wrong size: {name}")
    actual_hash = hashlib.sha256(path.read_bytes()).hexdigest()
    if actual_hash != expected_hash:
        raise SystemExit(f"Preserved v0.1.0 output has the wrong SHA-256: {name}")

fd_bytes = (root / "artifacts/release-v0.1.0/SALAA_EFI.release-v0.1.0.fd").read_bytes()
if fd_bytes[40:44] != b"_FVH":
    raise SystemExit("The preserved FD does not start with a PI firmware-volume header")
if int.from_bytes(fd_bytes[32:40], "little") != len(fd_bytes):
    raise SystemExit("The preserved FD volume length does not match the file size")

# Report known gaps. This is deliberately a source audit, not an EDK II build.
blockers = [
    ("SalaaPkg/Include/DXE.inc", "source/Platforms/Realme/SalaaPkg/Include/DXE.inc"),
    ("SalaaPkg/Include/SalaaMuDefaults.dsc.inc", "source/Platforms/Realme/SalaaPkg/Include/SalaaMuDefaults.dsc.inc"),
    ("MT6785Pkg/Library/MemoryMapLib/MemoryMapLib.inf", "source/Silicon/MediaTek/MT6785Pkg/Library/MemoryMapLib/MemoryMapLib.inf"),
    ("MT6785Pkg/Library/SerialPortLib/SerialPortLib.inf", "source/Silicon/MediaTek/MT6785Pkg/Library/SerialPortLib/SerialPortLib.inf"),
    ("MT6785Pkg/Drivers/ShellBootDxe/ShellBootDxe.inf", "source/Silicon/MediaTek/MT6785Pkg/Drivers/ShellBootDxe/ShellBootDxe.inf"),
]
summary = [
    "## SALAA UEFI source audit",
    "",
    "**Result:** repository/status claims, five preserved release-output hashes, and the FD PI-volume signature/length checked. No EDK II build or firmware execution was performed.",
    "",
    "Sanitized F.11/live-DT evidence and input hashes checked. The supplied EDK II archive does not close the SALAA source dependencies.",
    "",
    "Known build/compatibility blockers:",
]
for display, path in blockers:
    state = "present" if (root / path).exists() else "not supplied by this repository"
    summary.append(f"- `{display}` — {state}")
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
