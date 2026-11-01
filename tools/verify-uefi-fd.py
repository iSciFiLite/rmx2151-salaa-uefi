#!/usr/bin/env python3
"""Check the structural signature and expected extent of an EDK II FD."""
import hashlib
import json
import struct
import sys
from pathlib import Path


def main(path: str) -> None:
    fd = Path(path).read_bytes()
    if len(fd) != 0x400000:
        raise SystemExit(f"FD length is {len(fd)} bytes; expected 4194304")
    if fd[0x28:0x2C] != b"_FVH":
        raise SystemExit("PI firmware-volume signature missing at FD header offset 0x28")
    volume_length = struct.unpack_from("<Q", fd, 0x20)[0]
    if volume_length != len(fd):
        raise SystemExit(f"FD header length is {volume_length}; file length is {len(fd)}")
    if not any(fd):
        raise SystemExit("FD is empty")
    print(json.dumps({
        "file": Path(path).name,
        "size_bytes": len(fd),
        "firmware_volume_signature": "_FVH",
        "sha256": hashlib.sha256(fd).hexdigest(),
        "device_boot_tested": False,
    }, indent=2))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit(f"usage: {sys.argv[0]} SALAA_EFI.fd")
    main(sys.argv[1])
