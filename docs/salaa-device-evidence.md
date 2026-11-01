# SALAA device evidence supplied for UEFI bring-up

This record captures the non-secret facts extracted from the user-supplied
RMX2151 inputs. The original live DTB is deliberately not checked in: its
boot arguments contain a device serial number. The hashes below identify the
inputs without publishing that identifier or any firmware payload.

## Input inventory

| Input | Size | SHA-256 | What it contains |
|---|---:|---|---|
| `SALAA_F11_mainline_inputs.zip` | 108,172 bytes | `43af30c7dcf0ba0413c00bf000c3b2890e8a0325b285713a7e182b844292cf33` | F.11 metadata, base DTB, three DTBOs, and scatter files |
| `salaa-live.dtb` | 227,519 bytes | `0974ca677db2aaf3010b602d37cc123f33860a1d75e9f1fed735178f72e739ec` | Live Android DTB; analyzed locally, not published |
| `edk2_backup_20261026_183727.tar.gz` | 281,885,686 bytes | `5621aa51e029c58e19092a391d44e64cd6df502bef31cfc7738fb9fe363f1ee5` | Generic `edk2/` source tree; MD5 `05239c223c43e06bc92ef90eef12b0ed` |

The F.11 input manifest identifies RMX2151L1 / RM6785 / MT6785 and firmware
`RMX2151_11.F.11_3110_202211171630` (Android 12, vendor Linux 4.14.186).
These identify the supplied firmware inputs, not a confirmed live handset
build fingerprint.

## Live DT observations

- Root-compatible identifies MediaTek MT6785.
- `/memory` declares base `0x40000000`, size `0x200000000` (8 GiB).
- `/reserved-memory` contains 26 decoded `reg` ranges. This is vendor DT data, not a
  measured LK/UEFI memory map; reservations can overlap and their ownership
  must be established before constructing UEFI memory descriptors.
- The enabled UFS host node is at `0x11270000` with a `0x2300`-byte register
  range. Its enabled PHY node is at `0x11fa0000` with a `0xc000`-byte range.
  These addresses establish DT declarations only; they do not establish that
  the UEFI can initialize the controller, PHY, link, or expose Block I/O.
- The live tree lists six Cortex-A55 and two Cortex-A75 cores and uses PSCI in
  its CPU nodes. No UEFI PSCI call or secondary-core start was observed.
- The supplied firmware DT/DTBO manifest reports DSI at `0x14014000` and the
  panel/touch identifiers `truly,td4330,vdo` and `novatek,nf_nt36672c`.
  Neither a functioning GOP nor panel/touch initialization follows from those
  identifiers.

The current source still sets `PcdSystemMemorySize` to `0x3e605000`, which
does not match the live DT's 8 GiB `/memory` declaration. Do not simply change
it to 8 GiB: the UEFI-safe ranges, firmware/secure carve-outs, running image,
stacks, and framebuffer have not been correlated with the bootloader's memory
map. Resolve this against the actual SALAA SEC entry contract before selecting
FD, stack, or DXE heap addresses.

## EDK II archive result

The supplied 281,885,686-byte archive was downloaded and its SHA-256 and MD5
match the file metadata. Its archive root contains a generic `edk2/` source
tree. It does **not** contain the original Mu-Silicium workspace, the SALAA
package, `SalaaPkg/Include/DXE.inc`,
`SalaaPkg/Include/SalaaMuDefaults.dsc.inc`,
`MT6785Pkg/Library/MemoryMapLib`,
`MT6785Pkg/Library/SerialPortLib`, `MT6785Pkg/Drivers/ShellBootDxe`, or the
original successful build log. The separately supplied `fail.log` is a Linux
kernel `Image.gz` failure log, not an EDK II success log. The separate
`SALAA_F11_mainline_inputs.zip` contains device-tree and scatter inputs, not
those EDK II sources.

Therefore, these inputs improve hardware identification and expose a RAM-size
mismatch, but do not close source dependencies or make a hosted EDK II build
reproducible. The historical v0.1.0 FD remains preserved as a separate,
user-reported successful local build artifact; this evidence does not
reproduce or validate it.
