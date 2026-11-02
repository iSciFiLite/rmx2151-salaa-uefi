#!/usr/bin/env python3
"""Pack a one-shot SALAA UEFI Android boot image from verified F.11 inputs."""
from __future__ import annotations

import argparse
import gzip
import hashlib
import json
import re
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path

STOCK_BOOT_SIZE = 33_554_432
STOCK_BOOT_SHA256 = "27227c7345e412998ec7394dd37646adc5da8614f71d5653f747840fcdcc57c5"
FD_SIZE = 4 * 1024 * 1024
FD_BASE = 0x40000000
FD_SHA256 = "0cef01b241c42ae97dc2776e77e8bbe9431a7ad031eaaeed78f0036c59407c49"
SHIM_SHA256 = "6b72283e965a942d197d6721cdb7ebed43bbe17b6f45a371d93b97753ab1ef85"
KERNEL_ADDRESS = 0x40080000
RAMDISK_ADDRESS = 0x47C80000
DTB_ADDRESS = 0x4BC80000
BOOT_PARTITION_SIZE = 32 * 1024 * 1024


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def run(*argv: str) -> str:
    completed = subprocess.run(argv, check=True, text=True, stdout=subprocess.PIPE)
    return completed.stdout


def replace_option(args: list[str], option: str, value: str) -> None:
    try:
        index = args.index(option)
    except ValueError as exc:
        raise ValueError(f"unpack_bootimg did not return {option}") from exc
    if index + 1 >= len(args):
        raise ValueError(f"unpack_bootimg returned {option} without a value")
    args[index + 1] = value


def parse_info(info: str) -> None:
    expected = {
        "boot image header version": "2",
        "kernel load address": "0x40080000",
        "ramdisk load address": "0x47c80000",
        "kernel tags load address": "0x4bc80000",
        "dtb address": "0x000000004bc80000",
        "page size": "2048",
    }
    for label, value in expected.items():
        if not re.search(rf"^{re.escape(label)}:\s*{re.escape(value)}\s*$", info, re.M | re.I):
            raise ValueError(f"boot image {label} does not match SALAA F.11 ({value})")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stock-boot", required=True, type=Path)
    parser.add_argument("--fd", required=True, type=Path)
    parser.add_argument("--shim", required=True, type=Path)
    parser.add_argument("--unpack-bootimg", required=True, type=Path)
    parser.add_argument("--mkbootimg", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    parser.add_argument("--source-commit", required=True)
    parser.add_argument("--fd-release", required=True)
    args = parser.parse_args()

    if args.stock_boot.stat().st_size != STOCK_BOOT_SIZE or sha256(args.stock_boot) != STOCK_BOOT_SHA256:
        raise ValueError("stock boot image is not the verified SALAA F.11 input")
    if args.fd.stat().st_size != FD_SIZE:
        raise ValueError(f"SALAA FD must be exactly {FD_SIZE} bytes")
    if sha256(args.fd) != FD_SHA256:
        raise ValueError("SALAA FD does not match the verified vanilla firmware release")
    if not args.shim.is_file() or sha256(args.shim) != SHIM_SHA256:
        raise ValueError("BootShim does not match the verified SALAA BootShim release")

    args.out_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="salaa-bootimg-") as temp_name:
        temp = Path(temp_name)
        original = temp / "original"
        rebuilt = temp / "rebuilt"
        original.mkdir()
        rebuilt.mkdir()

        original_info = run(sys.executable, str(args.unpack_bootimg),
                            "--boot_img", str(args.stock_boot), "--out", str(original))
        parse_info(original_info)
        mk_args = run(sys.executable, str(args.unpack_bootimg),
                      "--boot_img", str(args.stock_boot), "--out", str(original),
                      "--format", "mkbootimg", "-0").encode().split(b"\0")
        if mk_args and not mk_args[-1]:
            mk_args.pop()
        command = [item.decode() for item in mk_args]

        kernel = temp / "salaa-uefi-kernel.gz"
        kernel.write_bytes(gzip.compress(args.shim.read_bytes() + args.fd.read_bytes(), mtime=0))
        replace_option(command, "--kernel", str(kernel))

        output = args.out_dir / "SALAA_UEFI_BOOT.img"
        run(sys.executable, str(args.mkbootimg), *command, "-o", str(output))
        if output.stat().st_size > BOOT_PARTITION_SIZE:
            raise ValueError("generated boot image exceeds the SALAA boot partition size")

        rebuilt_info = run(sys.executable, str(args.unpack_bootimg),
                           "--boot_img", str(output), "--out", str(rebuilt))
        parse_info(rebuilt_info)
        payload = gzip.decompress((rebuilt / "kernel").read_bytes())
        expected_payload = args.shim.read_bytes() + args.fd.read_bytes()
        if payload != expected_payload:
            raise ValueError("repacked kernel does not contain the exact BootShim followed by the SALAA FD")
        for component in ("ramdisk", "dtb"):
            if sha256(original / component) != sha256(rebuilt / component):
                raise ValueError(f"repacked stock {component} differs from the verified source")

        verification = {
            "image": output.name,
            "size_bytes": output.stat().st_size,
            "sha256": sha256(output),
            "stock_boot_sha256": STOCK_BOOT_SHA256,
            "fd_sha256": sha256(args.fd),
            "shim_sha256": sha256(args.shim),
            "ramdisk_sha256": sha256(rebuilt / "ramdisk"),
            "dtb_sha256": sha256(rebuilt / "dtb"),
            "header_version": 2,
            "page_size": 2048,
            "kernel_address": hex(KERNEL_ADDRESS),
            "fd_address": hex(FD_BASE),
            "fd_size": FD_SIZE,
            "boot_method": "one-shot fastboot boot; do not flash",
            "device_boot_tested": False,
        }
        (args.out_dir / "verification.json").write_text(json.dumps(verification, indent=2) + "\n")
        provenance = (
            "# SALAA one-shot UEFI boot image\n\n"
            f"- Date: {datetime.now(timezone.utc).isoformat()}\n"
            f"- SALAA source commit: `{args.source_commit}`\n"
            f"- Vanilla FD release: `{args.fd_release}`\n"
            f"- Stock F.11 boot input SHA-256: `{STOCK_BOOT_SHA256}`\n"
            "- Format: Android boot image v2, 2048-byte pages\n"
            f"- Kernel entry/load address: `{hex(KERNEL_ADDRESS)}`\n"
            f"- BootShim copies the {FD_SIZE}-byte FD to `{hex(FD_BASE)}`\n"
            "- Contents: BootShim + vanilla SALAA FD + matching stock F.11 ramdisk and DTB\n"
            "- Checks: stock input hash/header, output header/load parameters, exact shim/FD payload, ramdisk/DTB identity, image size\n"
            "- Start with `fastboot boot SALAA_UEFI_BOOT.img`; do not flash this image\n"
            "- Phone boot and recovery: not tested\n"
        )
        (args.out_dir / "PROVENANCE.md").write_text(provenance)
        sums = f"{sha256(output)}  {output.name}\n"
        (args.out_dir / "SHA256SUMS").write_text(sums)
    print(json.dumps(verification, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        raise SystemExit(f"pack-salaa-bootimg: {exc}") from exc
