#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit SALAA UEFI repository claims and preserved historical artifacts."""
from pathlib import Path
import hashlib
import os
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
readme = (ROOT / "README.md").read_text()
for phrase in (
    "Do not flash",
    "not hardware-tested",
    "UFS Block I/O",
    "Roadmap",
    "HakiOS",
    "reproducible",
):
    if phrase.lower() not in readme.lower():
        raise SystemExit(f"README is missing required status text: {phrase}")

evidence_path = ROOT / "docs/salaa-device-evidence.md"
if not evidence_path.is_file():
    raise SystemExit("The sanitized SALAA device-evidence report is missing")
evidence = evidence_path.read_text()
for phrase in (
    "0974ca677db2aaf3010b602d37cc123f33860a1d75e9f1fed735178f72e739ec",
    "0x200000000",
    "0x11270000",
    "0x11fa0000",
    "7df00000-7dffffff",
    "ro.product.model",
):
    if phrase.lower() not in evidence.lower():
        raise SystemExit(f"SALAA evidence report is missing expected device fact: {phrase}")
if re.search(r"androidboot\.serialno=[^\s\"']+|serialno\s*=\s*\"[^\"]+\"", evidence, re.I):
    raise SystemExit("A device serial value must not be committed in the evidence report")

required_files = (
    "source/Platforms/Realme/SalaaPkg/SalaaPkg.dsc",
    "source/Platforms/Realme/SalaaPkg/SalaaPkg.dec",
    "source/Platforms/Realme/SalaaPkg/SalaaPkg.fdf",
    "source/Platforms/Realme/SalaaPkg/DeviceBuild.py",
    "source/Platforms/Realme/SalaaPkg/Include/DXE.inc",
    "source/Platforms/Realme/SalaaPkg/Library/MemoryMapLib/MemoryMapLib.inf",
    "source/Platforms/Realme/SalaaPkg/Library/MemoryMapLib/MemoryMapLib.c",
    "source/Silicon/Silicium/SiliciumPkg/Drivers/SimpleFbDxe/SimpleFb.c",
    "source/Silicon/Silicium/SiliciumPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf",
    "source/Silicon/MediaTek/MT6785Pkg/Library/PlatformSecLib/PlatformSecLib.inf",
    "source/Silicon/MediaTek/MT6785Pkg/Library/PlatformSecLib/PlatformSec.c",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/PlatformSec.c",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/PlatformSec.inf",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/Sec.h",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/AArch64/ArchSec.c",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/AArch64/Exception.S",
    "source/Silicon/MediaTek/MT6785Pkg/Sec/AArch64/Helper.S",
    "Resources/Configs/Salaa.toml",
    ".github/workflows/edk2-build.yml",
    "tools/verify-uefi-fd.py",
    "tools/audit-memory-map.py",
    "tools/prepare-hakios-mu.sh",
    "tools/verify-hakios-integration.py",
    "integrations/hakios/lock.env",
    ".github/workflows/hakios-uefi-build.yml",
    ".github/workflows/salaa-bootimg-build.yml",
    ".github/workflows/salaa-hakios-xnu-build.yml",
    "tools/pack-salaa-bootimg.py",
)
for relative in required_files:
    if not (ROOT / relative).is_file():
        raise SystemExit(f"Required SALAA build input is missing: {relative}")

# Guard hardware details that were corrected from the owner-supplied pstore
# evidence and the standard UFSHCI status-register definition.
ufs_source = (ROOT / "source/Silicon/MediaTek/MT6785Pkg/Drivers/UfsBlockIoDxe/UfsBlockIoDxe.c").read_text()
for marker in (
    "#define HCS_DP                     BIT0",
    "#define HCS_UTRLRDY                BIT1",
    "#define HCS_UTMRLRDY               BIT2",
    "#define HCS_UCRDY                  BIT3",
    "((Hcs & HCS_DP) != 0)",
    "((Hcs & HCS_UCRDY) != 0)",
):
    if marker not in ufs_source:
        raise SystemExit(f"UFSHCI probe is missing the verified status-bit mapping: {marker}")
if "HCS bit 0 is UCRDY" in ufs_source:
    raise SystemExit("UFSHCI status bit 0 must not be labeled UCRDY")

gop_source = (ROOT / "source/Silicon/Silicium/SiliciumPkg/Drivers/SimpleFbDxe/SimpleFb.c").read_text()
for marker in ("*FrameBufferSize = FrameBufferRegion.Length;", "mFrameBuffer.Mode->FrameBufferSize = FrameBufferSize;"):
    if marker not in gop_source:
        raise SystemExit(f"SALAA GOP is not preserving its measured framebuffer allocation: {marker}")

workflow = (ROOT / ".github/workflows/edk2-build.yml").read_text()
if "4bd8ae83bf37de41777f19e0777db70f59383a6d" not in workflow:
    raise SystemExit("The hosted build is not pinned to the reviewed Mu-Silicium commit")
if "build_uefi.py -d Salaa -r DEBUG -c" not in workflow:
    raise SystemExit("The hosted workflow does not invoke the SALAA EDK II build")

hakios_workflow = (ROOT / ".github/workflows/hakios-uefi-build.yml").read_text()
for pin in (
    "687fa6c6a5a70f43184de92980e0a76eea5f2c4b",
    "e51cd877e31c8fbb322032dc4cb6715e83d25ad3",
):
    if pin not in hakios_workflow:
        raise SystemExit(f"The hosted HakiOS build is missing reviewed source pin: {pin}")
if "make -C mu/BootShim" not in hakios_workflow:
    raise SystemExit("The hosted HakiOS workflow does not build the SALAA BootShim")

bootimg_workflow = (ROOT / ".github/workflows/salaa-bootimg-build.yml").read_text()
for marker in (
    "27227c7345e412998ec7394dd37646adc5da8614f71d5653f747840fcdcc57c5",
    "untagged-3597467fa853612dd205",
    "v0.6.1-vanilla.fd5785f",
    "v0.3.0-hakios.8e6cf0d",
    "fastboot boot SALAA_UEFI_BOOT.img",
):
    if marker not in bootimg_workflow:
        raise SystemExit(f"The SALAA boot-image workflow is missing verified setting: {marker}")

xnu_workflow = (ROOT / ".github/workflows/salaa-hakios-xnu-build.yml").read_text()
for marker in (
    "iSciFiLite/hakios-mt6785",
    "f50d914",
    "kernel=hakios-kernel ramdisk=userland/hello",
    "SALAA_XNUP.bin",
    "v0.3.0-hakios.8e6cf0d",
    "fastboot boot",
):
    if marker not in xnu_workflow:
        raise SystemExit(f"The SALAA HakiOS/XNU workflow is missing required input or safety marker: {marker}")

memory_audit = subprocess.run(
    ["python3", str(ROOT / "tools/audit-memory-map.py")],
    check=False,
    capture_output=True,
    text=True,
)
if memory_audit.returncode:
    raise SystemExit(memory_audit.stderr or memory_audit.stdout)

try:
    tracked = subprocess.check_output(
        ["git", "-C", str(ROOT), "ls-files", "-z"], text=False
    ).decode().split("\0")
except subprocess.CalledProcessError as exc:
    raise SystemExit("This audit must run in a Git checkout") from exc
binary_suffixes = {".fd", ".efi", ".dll", ".map", ".pdb", ".img", ".bin"}
allowed_artifact_prefixes = ("artifacts/release-v0.1.0/", "artifacts/actions/")
committed_artifacts = [
    path for path in tracked
    if path and (Path(path).suffix.lower() in binary_suffixes or path.startswith("build/Build/"))
    and not path.startswith(allowed_artifact_prefixes)
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
    path = ROOT / "artifacts/release-v0.1.0" / name
    if not path.is_file() or path.stat().st_size != expected_size:
        raise SystemExit(f"Preserved v0.1.0 output is missing or has the wrong size: {name}")
    if hashlib.sha256(path.read_bytes()).hexdigest() != expected_hash:
        raise SystemExit(f"Preserved v0.1.0 output has the wrong SHA-256: {name}")

fd = (ROOT / "artifacts/release-v0.1.0/SALAA_EFI.release-v0.1.0.fd").read_bytes()
if fd[40:44] != b"_FVH" or int.from_bytes(fd[32:40], "little") != len(fd):
    raise SystemExit("The historical FD fails PI firmware-volume signature/length checks")

report = "\n".join((
    "## SALAA repository audit",
    "",
    "**Result:** required build files and sanitized device evidence are present; historical release hashes and PI-volume metadata match.",
    "",
    "This audit is not an EDK II build or hardware test. The separate pinned Mu-Silicium workflow compiles the firmware and checks the generated FD structure.",
    "",
    "Still requiring device evidence: exact LK FD load/entry contract, the omitted inverted System RAM interval, GOP scanout/pixel format, UART setup, UFS host/PHY/Block I/O, and actual boot/recovery behavior.",
    "",
))
summary = os.environ.get("GITHUB_STEP_SUMMARY")
if summary:
    with open(summary, "a", encoding="utf-8") as stream:
        stream.write(report)
print(report)
