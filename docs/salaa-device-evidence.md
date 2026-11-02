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

The 2026-11-02 file named `SALAA_F11_mainline_inputs.tar` was identified as a
ZIP archive by its file signature. Its SHA-256 matches the F.11 ZIP already
listed above. The archive contains only a markdown manifest, `build.prop`, two
scatter descriptions, one base DTB, and three DTBOs; it contains no OFP, boot
partition image, ramdisk, `lk.bin`, or `lk2.bin`. No full OFP extraction was
performed.

The F.11 manifest identifies RMX2151L1 / RM6785 / MT6785 and firmware
`RMX2151_11.F.11_3110_202211171630` (Android 12, vendor Linux 4.14.186).
This is the supplied firmware package identity; the live handset reports a
different build fingerprint below.

The bundled `metadata/build.prop` independently identifies stock F.11 as
`RMX2151_11_F.11` / Android 12, model `RMX2151`, with
`ro.build.fingerprint=realme/RMX2151/RMX2151L1:12/SP1A.210812.016/Q.bf75e7-1:user/release-keys`.
This is static stock-firmware identity available from the supplied bundle;
it does not identify the currently booted custom ROM. The separate live
Termux snapshot reports another Realme fingerprint, and `commands-info.txt`
reports that same Android 12 fingerprint despite the owner's current-ROM
description of Android 16 QPR2. That discrepancy does not require another
fingerprint request for the UEFI work.

### Selective F.11 boot-chain inspection — 2026-11-02

The Ubuntu data volume contains the 10,546,675,416-byte
`RMX2151_F11.ofp` and an existing `selected_f11/` extraction. This turn did
not decrypt or extract the whole OFP. We inspected the already-extracted
boot-chain files only. The 33,554,432-byte `boot.img` SHA-256 is
`27227c7345e412998ec7394dd37646adc5da8614f71d5653f747840fcdcc57c5`, exactly
matching the supplied manifest. AOSP `unpack_bootimg.py` from
`platform/system/tools/mkbootimg` commit
`d2bb0af5ba6d3198a3e99529c97eda1be0b5a093` (script SHA-256
`a9d260978a63bd06a24b6347e7dee8a28ff96639793caea15dff6aa491316308`) parsed
the actual binary as Android boot header v2, page size 2048, kernel size
15,662,146 at `0x40080000`, ramdisk size 1,434,484 at `0x47c80000`, and DTB
size 141,677 at `0x4bc80000`; the command line is
`bootopt=64S3,32N2,64N2 buildvariant=user`. The extracted `dtb`, `kernel`, and
`ramdisk` hashes are respectively
`ddcb4f21c19f203b36547b5f0e5663399361f954fb884114c39c775db68b9ccc`,
`c3ebf513d0ab3e29d5ac9a3fc17b486a85fe6a4510307395ee6d734b20d1fb89`, and
`2f8306faaf3c73851b31fe9c6e71c4cb6fb290f316165bbb91a6a48a4a5d5ba4`.

The same extraction has `lk.img` and `lk2.img` (each 1,387,824 bytes, equal
SHA-256 `d532ce63f1fefb02b39a5f32048d7d2b67af5cc17d7b8d4424beed6b4cf58ca0`),
plus the 375,120-byte preloader. These boot-chain binaries are not committed
or released. The LK images are not needed to assemble a temporary Android
boot image and must not be patched or flashed as part of this effort. The
running phone is now on a custom Android 16 QPR2 ROM; these F.11 files are
stock-package references, not proof that its current `boot` partition is
identical.

As a cross-check, the supplied device-tree repository's public
[`lineage-23.2/BoardConfig.mk`](https://github.com/elohim-etz/android_device_realme_RM6785/blob/lineage-23.2/BoardConfig.mk)
sets header v2, 2048-byte pages, kernel base `0x40078000` plus offset
`0x8000` (effective `0x40080000`), ramdisk offset `0x07c08000`, and DTB/tags
offset `0x0bc08000`. This corroborates the parsed F.11 header layout for that
Android build configuration; it does not verify that the phone's current
custom-ROM boot partition is identical.

The existing UEFI FD and BootShim are not themselves an Android boot image.
HakiOS's upstream `mkxnuimg.py` describes the fastboot kernel payload as
`gzip(BootShim + UEFI FD + optional XNUP) + DTB` and calls `fastboot boot`;
its checked-in defaults target Redwood (2 MiB FD at `0x9FD00000`, Android
header v1), so those device values cannot be reused for SALAA. The SALAA
packer preserves the matching F.11 Android v2 wrapper, ramdisk, and separate
DTB, replacing only the kernel with `gzip(BootShim + 4 MiB vanilla SALAA FD)`.
The hosted UEFI-entry output is published at
[`v0.7.0-bootimg.41512150665`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.0-bootimg.41512150665)
(1,951,744 bytes; SHA-256
`59a416bb6e99884ee1401f0cd20d3e5440bfc31a6e8fcc72eef169a4241b75d3`). It
contains no XNUP/XNU payload and uses the vanilla FD; it is a diagnostic, not
the intended HakiOS/XNU image. The F.11 input is held in a draft-only GitHub
release asset and downloaded by Actions with the repository token; it is not
in Git or a public input release. A separate hosted workflow now builds the
pinned MT6785 SALAA XNU kernel and open `hello` ramdisk, packages them as XNUP,
and combines that with the matching HakiOS SALAA FD + BootShim. Hosted run
`41517036914` was the first pinned-toolchain candidate; it exposed a release
shell continuation defect after uploading the main assets. The fix was applied,
and clean end-to-end run `41520513335` passed both build and package jobs. Its
image, XNUP, kernel, ramdisk, toolchain hash, verification, and provenance are
released at
[`v0.7.1-hakios-xnu.41520513335`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.1-hakios-xnu.41520513335).
The complete release was downloaded and every item in `SHA256SUMS` verified.
Either image is for `fastboot boot` testing only, not a flashable `boot`
partition image. LK modification is neither required nor planned.

## Current handset evidence

The owner ran these commands in Termux as root and supplied the output:

| Query | Result |
|---|---|
| `ro.product.model` | `RMX2151L1` |
| `ro.build.fingerprint` | `realme/RMX2156T2/RMX2156L1:12/SP1A.210812.016/Q.174ebd4_fa4d:user/release-keys` |
| `MemTotal` | 7,830,224 kB (about 7.46 GiB Linux-visible) |
| Live DT `/memory` | Base `0x40000000`, size `0x200000000` (8 GiB physical span) |
| `/sys/class/graphics/fb0/name` | `mtkfb` |
| `/sys/class/graphics/fb0/bits_per_pixel` | `32` |
| `/sys/class/graphics/fb0/virtual_size` | `1088,9600` |
| `/sys/class/graphics/fb0/modes` | `U:1080x2400p-0` |
| `/sys/class/graphics/fb0/line_length`, `smem_start`, `smem_len` | No values returned by the owner's query |

The fingerprint names RMX2156T2/RMX2156L1 while `ro.product.model` says
RMX2151L1. Keep the profile restricted to this observed hardware until the
regional/model relationship is confirmed; do not assume it covers every
SALAA/Narzo 20 Pro variant.

### Framebuffer, kernel log, and partition aliases — owner output, 2026-11-02

The owner ran these root Termux queries:

```sh
su -c 'for f in name bits_per_pixel line_length virtual_size smem_start smem_len format; do printf "%s=" "$f"; cat "/sys/class/graphics/fb0/$f" 2>/dev/null; done; find /sys/class/graphics/fb0 -maxdepth 1 -type f -printf "%f\n" 2>/dev/null | sort'
su -c 'dmesg 2>/dev/null | grep -Ei "ufshci|ufs phy|ufs|mtkfb|framebuffer|fb0|td4330|nt36672c" | tail -100'
su -c 'find /dev/block/by-name -maxdepth 1 -type l -printf "%f -> %l\n" 2>/dev/null | grep -Ei "xbl|lk|boot|dtbo|log|ufs"'
```

Reported results: framebuffer name `mtkfb`, 32 bits per pixel, virtual size
`1088,9600`; no values were printed for `line_length`, `smem_start`,
`smem_len`, or `format`. The filtered `dmesg` query printed no matching
lines. The by-name links shown were `dtbo -> /dev/block/sdc45`,
`lk2 -> /dev/block/sdc42`, `boot_para -> /dev/block/sdc28`,
`lk -> /dev/block/sdc41`, `logo -> /dev/block/sdc44`, and
`boot -> /dev/block/sdc43`. These are observations from the running ROM,
not proof of the UEFI boot contract, UFS LUN mapping, or permission to write
those partitions. The combined `printf` output has no newline between fields,
which explains why empty fields run together in the pasted terminal output.

### Sanitized follow-up snapshot — owner attachment, 2026-11-02

The owner supplied `commands-info.txt` (262,449 bytes). The raw file is not
committed because its pstore boot command line contains the handset serial.
Only relevant, sanitized results are retained:

| Read-only source | Reported result | What it establishes / does not establish |
|---|---|---|
| Android properties | `RMX2151L1`; fingerprint `realme/RMX2156T2/RMX2156L1:12/...`; `ro.boot.flash.locked=1`; `ro.boot.vbmeta.device_state=locked`; `ro.boot.verifiedbootstate=green`; `ro.bootmode=normal` | Identifies the handset properties in this snapshot. The fingerprint says Android 12 while the owner describes the currently used ROM as Android 16 QPR2 Pixel-based; this mismatch is unresolved and properties can be inherited or overridden by a ROM. Do not infer bootloader state from these properties alone. |
| `/sys/class/scsi_host/host0/proc_name` | `ufshcd` | Android's kernel exposes a UFS host. It is not evidence that the UEFI Block I/O driver works. |
| pstore `console-ramoops` display log | Framebuffer PA start `0x75d30000`, end `0x789fffff` (inclusive) | Matches the previously decoded `0x02cd0000` allocation size and corroborates the Linux-side reservation. Does not prove GOP pixel format, scanout after handoff, or framebuffer lifetime. |
| pstore UFS log | MTK UFSHCI at `0x11270000`; link changes from gear 1 slow-auto to gear 3 fast mode, one lane each direction, rate 2; Samsung `KM8V8001JM-B813`; direct-access SCSI LUNs 0 and 1 appear | Confirms the Android vendor kernel initialized this physical UFS device and enumerated media. It supplies reference behavior for a future UEFI driver; it does not implement or validate UEFI UFS PHY/link/transfer support. |

The snapshot's `flash.locked=1` and `vbmeta.device_state=locked` make the
fastboot preflight especially important. Confirm the bootloader state from
the external fastboot host before considering a RAM boot. No flash command
is authorized or needed.

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
layout, the FD must replace the Android kernel payload. It also excludes the
HakiOS XNUP window `0xF0000000..0xF3FFFFFF` from DXE allocations, matching the
upstream BootShim's fixed payload address. The F.11 scatter
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

The owner later queried `/sys/class/graphics/fb0` as root. A virtual width of
1088 for a visible width of 1080 means the GOP scan-line pitch must describe
1088 pixels, not 1080. At 32 bpp that would be 4352 bytes per line if the
driver's pixel storage is four bytes per pixel; `line_length` was empty, so
4352 remains an inference. The `virtual_size` height of 9600 is four 2400-row
buffers; the GOP visible mode remains 1080×2400. The SALAA overlay now passes
the observed 1088-pixel stride to SimpleFb and checks that the selected mode
fits the DT-declared display reservation. The physical framebuffer start,
channel order, actual line length, and validity across the firmware handoff
still need phone evidence.

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
macro plugin before any C compilation because the SALAA platform directory
also lacked a DEC package descriptor. After adding `SalaaPkg.dec`, a later run
advanced past that check but failed because the DSC pointed to
`Platforms/Realme/SalaaPkg/Library/PlatformSecLib/PlatformSecLib.inf`, which
does not exist. The existing MT6785 package INF is now selected and the
SALAA-specific no-op implementation is colocated with it. The fourth hosted
run passed source-path validation and reached EDK library dependency
resolution; it then found missing mappings for standard Shell library
classes. The DSC now maps those classes to upstream EDK II implementations.
The sixth hosted run completed `build_uefi.py` and passed the corrected output-path
lookup, PI firmware-volume signature/length checks, and Actions upload. Run
[41260823722](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41260823722)
built source commit `22554c432ba6ef7eca59c4733885ff30a56a7686` with Mu-Silicium
commit `4bd8ae83bf37de41777f19e0777db70f59383a6d`. The 4 MiB FD SHA-256 is
`4d0bdc13590f925d7558c923413bc4013f1107e3d4828180730594ebf74eb646`. It is
available in the [hosted-build prerelease](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.2.0-actions.22554c4)
with verification metadata and logs. This verifies compile and image structure
only, not SEC entry, memory reservations, GOP, UFS, PSCI, recovery, or booting
an OS.

On 2026-11-01, a separate pinned HakiOS integration path was added. It checks
out HakiOS UEFI commit `687fa6c6a5a70f43184de92980e0a76eea5f2c4b` and its
Mu-Silicium base `e51cd877e31c8fbb322032dc4cb6715e83d25ad3`, applies the upstream
patch/files/banner procedure, overlays the SALAA platform, builds the HakiOS
FD, then compiles BootShim with the SALAA 4 MiB FD and 64 MiB XNUP constants.
The result and exact run status are linked from the top-level README. Even a
passing job verifies compilation only; no SALAA fastboot image or phone boot
has been tested. The UFS Block I/O and boot contract gaps remain.

## Firmware image evidence — 2026-11-01 (F.11 OFP extraction)

Extracted from `selected_f11/` directory produced by the OFP decryption step
run in a previous session:

### Scatter file (MT6785_20682_Android_scatter.txt)

Flash partition table for `project: k85v1_64`, `platform: MT6785`.  Relevant
entries (flash offsets, not DRAM load addresses):

| Partition | Flash offset | Size |
|-----------|-------------|------|
| preloader | 0x0 | 0x80000 |
| lk        | 0x56500000 | 0x200000 |
| lk2       | 0x56700000 | 0x200000 |
| boot_a    | 0x56900000 | (varies) |

> The `linear_start_addr` field in the scatter file is the **eMMC flash byte
> offset**, not a DRAM address.  It does not establish where LK copies itself
> to DRAM or where it jumps.

### LK image header (lk.img, 1.4 MiB)

Magic bytes: `88 16 88 58` = `0x58881688` — MediaTek LK2 header magic.
Second block at `0x30`: magic `0x58891689`, version `0x200`, `part_count=1`.
The payload section of the LK2 format has its `load_addr` field set to
`0xFFFFFFFFFFFFFFFF` in this image (standard for OFP-encrypted LK2 where the
actual load address is embedded in the preloader's partition-management table,
not in the LK header itself).

**Conclusion:** The LK DRAM load address, entry PC, exception level, SP, and
argument registers cannot be determined from this image alone without a
preloader log or UART capture.  The standard MT6785 LK2 bootloader (k85v1_64)
is believed to copy LK to DRAM at approximately `0x4C000000` and enter at
`+0` in AArch64 EL1 (based on MT6765/MT6771 public LK2 port behaviour), but
this is NOT confirmed for SALAA and must not be assumed safe.

### UART evidence (stock-salaa-1.dts)

```
serial@11002000 {
    compatible = "mediatek,mt6577-uart";
    reg = <0x00 0x11002000 0x00 0x1000>;
    interrupts = <0x00 0x73 0x08>;
    clocks = <0x13 0x21 0x15>;       /* baud clock (26 MHz ref), bus clock */
    clock-names = "baud\0bus";
    pinctrl-0 = <gpio53_rx>;
    pinctrl-1 = <gpio54_tx>;         /* GPIO53=RX, GPIO54=TX */
    status = "okay";
};
aliases: apuart0 = "/serial@11002000";
```

The `mediatek,mt6577-uart` IP is the same 8250-compatible UART block used
across MT6577, MT6795, MT6771, and MT6785. Register layout verified against
`drivers/tty/serial/8250/8250_mtk.c` from the device kernel tree.

**Baud-rate parameters for 115200 from 26 MHz (MTK high-speed mode 3):**

| Register | Offset | Value | Derivation |
|----------|--------|-------|------------|
| HIGHS    | 0x09*4 | 3     | Enable MTK fractional-divider mode 3 |
| DLL      | 0x00*4 | 1     | Divisor = ceil(26e6 / (256*115200)) = 1 |
| DLM      | 0x01*4 | 0     | High byte of divisor |
| SAMPLE_CNT | 0x0A*4 | 225 | 26e6/(115200*1) − 1 = 225 |
| SAMPLE_PT  | 0x0B*4 | 111 | SAMPLE_CNT/2 − 1 = 111 |

The `MT6785SerialPortLib` at
`Silicon/MediaTek/MT6785Pkg/Library/SerialPortLib/SerialPortLib.c`
implements this sequence. The previous null stub (`BaseSerialPortLibNull`) is
replaced; the CI build verifies the new lib compiles successfully.

### UFS evidence (stock-salaa-1.dts)

```
ufshci@11270000 {
    compatible = "mediatek,ufshci";
    reg = <0x00 0x11270000 0x00 0x2300>;
    interrupts = <0x00 0x68 0x08>;
    clocks = <ufs0-clock, ufs0-unipro-clk, ufs0-mp-clk, ufs0-aes-clk>;
};
ufs_mphy@11fa0000 {
    compatible = "mediatek,ufs_mphy";
    reg = <0x00 0x11fa0000 0x00 0xc000>;
};
```

The probe-only UFS stub at
`Silicon/MediaTek/MT6785Pkg/Drivers/UfsBlockIoDxe/` samples VER at 0x08, HCE at
0x34, HCS at 0x30, CAP at 0x00, and MTK_HW_VER at 0x2240. The JEDEC/Arm/Linux
UFSHCI mapping is `DP=HCS[0]`, `UTRLRDY=HCS[1]`, `UTMRLRDY=HCS[2]`, and
`UCRDY=HCS[3]`; older project documentation incorrectly assigned UCRDY to bit
0 and has been corrected. The probe now reports these states separately but
does not use DP alone to claim a LUN is usable. Optional Block I/O handles
still report no media; reads return `EFI_NO_MEDIA`. There is no UFS link
initialization, LUN discovery, capacity discovery, or block transfer.
Partition enumeration and XnuLoader persistence remain unsupported.
The bit mapping follows the [Arm Trusted Firmware UFS register definitions](https://github.com/ARM-software/arm-trusted-firmware/blob/master/include/drivers/ufs.h)
and the upstream [Linux UFS host-controller implementation](https://github.com/torvalds/linux/blob/master/drivers/ufs/core/ufshcd.c).

### Live GOP Framebuffer Hardware Evidence (`atag,videolfb`)

Extracted and verified from the phone's live device tree (the owner-provided live DTB)
node `/chosen/atag,videolfb`:

- **Physical Framebuffer Base (`fb_base`)**: `0x75d30000`
- **VRAM Reserved Length (`vramSize`)**: `0x02cd0000` (46.81 MiB)
- **LCM Status**: `islcmfound = 1`
- **Refresh Rate**: `fps = 6015` (60.15 Hz)
- **Panel Model**: `nt36672c_tianma_sala` (Tianma NT36672C, 1080×2400 @ 32 bpp)
- **Calculated byte pitch**: `1088 * 4 = 4352` bytes per line (matches rooted `fb0` virtual size `1088,9600`)

This proves that the physical framebuffer memory is mapped at `0x75d30000`–`0x789fffff`,
which exactly aligns with the `Display Reserved` descriptor in `MemoryMapLib.c`.
The `PcdFrameBufferBase` and `PcdFrameBufferSize` PCDs in `SalaaPkg.dec` / `SalaaPkg.dsc`
now lock in this exact address.

The SALAA SimpleFb override reports the complete `0x02cd0000` reserved range
as GOP `FrameBufferSize`; GOP mode information remains the visible
1080×2400 surface with 1088 pixels per scan line. This matches the Linux
allocation evidence without claiming that GOP scanout, pixel order, or
post-handoff display lifetime has been tested on the phone.
