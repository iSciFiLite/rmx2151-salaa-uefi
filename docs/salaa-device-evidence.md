# SALAA device evidence for UEFI bring-up

This report records sanitized facts from the user-supplied RMX2151 inputs and
the current phone's Termux output. The original live DTB is not committed:
its boot arguments contain a device serial number.

## Input inventory

| Input | Size | SHA-256 | Contents |
|---|---:|---|---|
| `SALAA_F11_mainline_inputs.zip` | 108,172 bytes | `43af30c7dcf0ba0413c00bf000c3b2890e8a0325b285713a7e182b844292cf33` | F.11 metadata, base DTB, 3 DTBOs, scatter files |
| `salaa-live.dtb` | 227,519 bytes | `0974ca677db2aaf3010b602d37cc123f33860a1d75e9f1fed735178f72e739ec` | Live Android DTB; analyzed locally, not published |
| `edk2_backup_20261026_183727.tar.gz` | 281,885,686 bytes | `5621aa51e029c58e19092a391d44e64cd6df502bef31cfc7738fb9fe363f1ee5` | Generic `edk2/` tree, not a Mu-Silicium workspace; MD5 `05239c223c43e06bc92ef90eef12b0ed` |

The F.11 manifest identifies RMX2151L1 / RM6785 / MT6785 and firmware
`RMX2151_11.F.11_3110_202211171630` (Android 12, vendor Linux 4.14.186).
This is the supplied firmware package identity; the live handset reports a
different build fingerprint below.

## Current handset evidence

The owner ran these commands in Termux as root and supplied the output:

| Query | Result |
|---|---|
| `ro.product.model` | `RMX2151L1` |
| `ro.build.fingerprint` | `realme/RMX2156T2/RMX2156L1:12/SP1A.210812.016/Q.174ebd4_fa4d:user/release-keys` |
| `MemTotal` | 7,830,224 kB (about 7.46 GiB Linux-visible) |
| Live DT `/memory` | Base `0x40000000`, size `0x200000000` (8 GiB physical span) |

The fingerprint names RMX2156T2/RMX2156L1 while `ro.product.model` says
RMX2151L1. Keep the profile restricted to this observed hardware until the
regional/model relationship is confirmed; do not assume it covers every
SALAA/Narzo 20 Pro variant.

The supplied `/proc/iomem` exposes these discontiguous System RAM intervals:

```text
40000000-4cdfffff
4d100000-55ffffff
56720000-7531ffff
75330000-75d2ffff
7df00000-7dffffff   # reported exactly this way; end is below start
7e010000-7ee7ffff
7f400000-7f4bffff
7f5c0000-7f5fffff
80000000-8bffffff
8cd00000-8dffffff
8e900000-8effffff
8f320000-9f5effff
9ff00000-bbffffff
bc400000-bcffffff
bd300000-bd513fff
c0000000-ffcfffff
ffe00000-13fffefff
140000000-23fffdfff
```

The invalid `0x7df00000..0x7dffffff` interval is deliberately omitted from
`MemoryMapLib.c`; its intended endpoint cannot be inferred safely. The user
reran `grep 'System RAM$' /proc/iomem` and confirmed the same malformed line.
No range is fabricated for it. The System RAM map spans 7.76 GiB, consistent
with the 8 GiB live DT plus reserved holes and Linux-visible accounting.

`/proc/iomem` also shows the stock Android kernel normally uses `0x40080000`
through `0x4261ffff` for kernel code and `0x42aa0000` through `0x434b7fff` for
kernel data. The current UEFI map can reuse that memory only in an alternate
boot where LK replaces the kernel with the UEFI payload. It must not coexist
with the stock kernel.

## Memory-size reconciliation

The supplied F.11 **base DTB** declares base `0x40000000`, size `0x3e605000`
(about 998 MiB). The user's **live Android DTB** declares size
`0x200000000` (8 GiB); Termux reports 7,830,224 kB `MemTotal`, and
`/proc/iomem` System RAM extents independently reach `0x23fffdfff`. These
live observations agree, so the platform's `PcdSystemMemorySize` is now
`0x200000000`. The older/base F.11 DT is not used as the live RAM size.

This resolves the RAM-*capacity* discrepancy for this running device; it does
not establish the pre-OS UEFI handoff. The `MemoryMapLib` uses the supplied
System RAM ranges, excludes the malformed line, reserves the selected FD and
stack windows, and defines a DXE heap in the first RAM bank. These static
addresses are still a candidate profile, not proof that the bootloader leaves
them available when it launches the FD.

The current source carries the historic FD base `0x40000000`, FD size 4 MiB,
and stack base `0x40400000`. Since these overlap the stock Android kernel
layout, the FD must replace the Android kernel payload. The F.11 scatter
addresses (`LK=0x56500000`, `LK2=0x56700000`, `boot=0x56900000`,
`DTBO=0x59100000`) are flash offsets and are not DRAM load addresses. No
provided log confirms the FD load address, entry PC, exception level, SP,
x0-x3 arguments, or device-tree pointer.

Before claiming a safe device boot, collect an LK/bootloader log or equivalent
showing the load/entry addresses and register handoff, plus the pre-OS
reserved ranges. `/proc/iomem` describes the running Android kernel's view,
not LK's pre-UEFI reservation state.

## Other live device-tree observations

- Root compatible identifies MediaTek MT6785.
- The enabled UFS host is at `0x11270000` with a `0x2300`-byte register
  range; the PHY is at `0x11fa0000` with a `0xc000`-byte range. These are
  declarations, not proof of clocks, PHY sequence, link training, DMA, or
  Block I/O.
- The enabled CPU nodes describe six Cortex-A55 and two Cortex-A75 cores and
  PSCI via SMC. UEFI PSCI calls and secondary CPU startup have not been run.
- The live tree contains 26 `/reserved-memory` `reg` ranges. A display
  reservation begins at `0x75d30000` and spans `0x02cd0000` bytes. The tree
  reports a 1080x2400-class panel; GOP pixel layout, stride, DSI ownership,
  and framebuffer lifetime still need device verification.
- The live tree reports DSI at `0x14014000`, panel `truly,td4330,vdo`, and
  touch controller `novatek,nf_nt36672c`. No DSI/panel/touch driver is
  included in the current UEFI package.

These are hardware descriptions only. They do not establish a working UEFI
driver or interface.

## EDK II source and hosted-build status

The supplied 281,885,686-byte archive was downloaded and hash-checked. It is
a generic `edk2/` source tree; it does not contain the original Mu-Silicium
workspace, its pinned submodules, the original successful build log, or the
SALAA-specific package files. The separately supplied SALAA zip contains
DTB/DTBO/scatter evidence, not EDK II implementation sources. The supplied
`fail.log` is an Android kernel `Image.gz` compile failure, not an EDK II
build log.

To make a clean hosted compile possible, this repository now includes:

- A SALAA DEC, `DeviceBuild.py`, and Mu config, using Mu-Silicium pinned at
  `4bd8ae83bf37de41777f19e0777db70f59383a6d`.
- A SALAA-local DXE firmware-volume list and an explicit set of generic EDK II
  modules; the absent custom `ShellBootDxe` is replaced by standard BDS/Shell.
- SEC helper files imported from the pinned Mu-Silicium `SiliciumPkg` with
  their BSD-2-Clause-Patent notices preserved.
- A SALAA memory-map library built from the user-provided live memory ranges.
- A no-op platform SEC hook; no undocumented MediaTek watchdog/clock register
  is written.
- A GitHub Actions workflow that overlays SALAA into the pinned recursive Mu
  workspace, installs upstream build dependencies, invokes the EDK II build,
  verifies FD structure, and preserves the output/logs as Actions artifacts.

The first hosted build reached Mu setup and failed in the pre-build debug
macro plugin before any C compilation because the platform directory lacked a
DEC package descriptor. `SalaaPkg.dec` has now been added; the next hosted
run must confirm package discovery and reveal any further EDK II errors. A
successful compile will only prove source/toolchain integration. It will not
verify SEC entry, memory reservations, GOP, UFS, PSCI, recovery, or booting an
OS.
