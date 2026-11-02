# SALAA UEFI — Realme 7 / RMX2151 / MT6785

**Status updated 2026-11-01.** This repository contains a SALAA EDK II platform overlay for [Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium), plus a separate, pinned [HakiOS UEFI](https://github.com/oosexgang/hakios-uefi) integration path. The source has compiled in GitHub Actions as a 4 MiB PI firmware volume. That proves source closure and image structure; it does **not** prove safe entry, phone boot, peripheral operation, or XNU boot.

> **Do not flash or chainload these images yet.** The LK load address, entry level/registers, complete pre-OS memory map, and a SALAA recovery procedure are not confirmed. The observed FD base overlaps Android's normal kernel load area, so any eventual test must replace the kernel payload and have a verified recovery path.

## Support and verification status

| Area | Implemented now | Evidence and remaining limit |
|---|---|---|
| SALAA platform | AArch64 SEC/DXE, board package, memory descriptors, SimpleFb GOP, standard EDK II BDS/Shell | The vanilla Mu-Silicium source compiled on hosted Actions; not hardware-tested on phone. |
| Mu-Silicium | Pinned workspace overlays and reproducible package configuration | Vanilla pin `4bd8ae83bf37de41777f19e0777db70f59383a6d` passed hosted build run [41260823722](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41260823722). HakiOS uses its own base pin, below. |
| HakiOS UEFI | Upstream BootShim/XNUP and BootManager/XnuLoader patch stack; SALAA overlay; separate hosted build and BootShim job | Build result is recorded in the latest [HakiOS integration Actions run](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/workflows/hakios-uefi-build.yml). A successful compile is source integration only. It does not make the phone runtime-compatible. |
| HakiOS/XNU fastboot handoff | FD layout matches the SALAA profile; XNUP window is reserved; workflow builds the AArch64 shim | Still needs a valid SALAA Android boot image/DTB/ramdisk, confirmed LK handoff, and hardware logs. The upstream XNU guide describes the payload at `0xF0000000` and a 64 MiB region; it does not document a SALAA test. |
| GOP/framebuffer | Uses live-DT display reservation and reports 1080×2400, 32 bpp, **1088 pixels per scan line** | Rooted `fb0` output reports `mtkfb`, 32 bpp, virtual size `1088,9600`, and mode `U:1080x2400p-0`. Base address, actual byte pitch, pixel channel order, and lifetime across UEFI handoff still need confirmation. The DSI/panel is not initialized by this GOP driver. |
| Internal UFS | No MT6785 UFS host/PHY initialization or UFS Block I/O driver | Unsupported. Device-tree addresses alone do not implement clocks, reset, PHY/link setup, DMA/IOMMU, or Block I/O. HakiOS's XnuLoader uses Block I/O to find the `logdump` partition; that diagnostic path cannot persist logs until UFS is working. |
| UART/serial | Null SerialPortLib; no speculative MMIO writes | No UART console. Need UART base, clock, pins, baud, and bootloader handoff evidence before enabling a physical serial driver. |
| USB | Generic USB bus, keyboard, and mass-storage class modules are included | They need a USB host-controller driver and board power/role setup, which are absent. |
| CPU, GIC, timer, reset | Generic Arm drivers and the live-tree MT6785 GIC/core declarations | SMC/PSCI behavior, timer calibration, secondary CPU start, and reset behavior are not tested. |
| Secure boot / variables | Unsigned bring-up configuration, runtime variable and capsule modules | Secure Boot is disabled in this experimental configuration. Persistent variable storage and capsule update are not validated. |

**Compatibility statement:** the SALAA source is laid out for Mu-Silicium and now has a hosted HakiOS-patched Mu build path. HakiOS's patch provides a source-level XNUP/BootManager integration. Neither the plain FD nor patched FD has been run on SALAA, so neither is yet verified compatible with HakiOS or XNU at runtime. Standard UEFI applications also need a UEFI-readable volume; this platform currently has no UFS Block I/O, so it cannot yet load `\\EFI\\BOOT\\BOOTAA64.EFI` from internal storage.

HakiOS documentation distinguishes a loader started from a UEFI volume from its `fastboot boot` shim/payload route. Its current UEFI repository supplies an XnuLoader integration and says its phone-specific vendor drivers are not redistributed; its XNU boot guide describes a `0xF0000000` payload window. We target the RAM/fastboot route first, but the SALAA Android boot image and on-device handoff remain unverified. See [HakiOS UEFI](https://github.com/oosexgang/hakios-uefi), [HakiOS XNU UEFI boot notes](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/docs/uefi-boot.md), and [the XNU image packer](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/mkxnuimg.py). The upstream packer defaults to another device's FD and DTB; do not use it unmodified for SALAA.

## Device identity and evidence

The profile is based on the actual rooted phone evidence supplied by its owner, not on a generic MT6785 assumption. The full sanitized inventory is in [`docs/salaa-device-evidence.md`](docs/salaa-device-evidence.md); the raw live DTB is not published because its boot arguments contain a serial.

| Observation | Value |
|---|---|
| `ro.product.model` | `RMX2151L1` |
| Build fingerprint | `realme/RMX2156T2/RMX2156L1:12/SP1A.210812.016/Q.174ebd4_fa4d:user/release-keys` |
| Linux `MemTotal` | 7,830,224 kB |
| Live DT memory | Base `0x40000000`, size `0x200000000` (8 GiB physical span) |
| Live-DT UFS declarations | HCI `0x11270000`, PHY `0x11fa0000`; presence is not a working UEFI storage driver |
| Live-DT display reservation | `0x75d30000`, length `0x02cd0000` |
| Rooted framebuffer sysfs | `mtkfb`, 32 bpp, virtual size `1088,9600`, mode `1080x2400` |
| Historical FD layout | Base `0x40000000`, size 4 MiB; stack begins at `0x40400000` |

The supplied F.11 *base* DTB reports a different RAM size than this device's live DT, `/proc/iomem`, and `MemTotal`. The current 8 GiB upper bound follows the live evidence. `/proc/iomem` contains the malformed line `7df00000-7dffffff : System RAM`; the user reran a filtered query and confirmed that exact invalid range. It is excluded, not guessed. The whole pre-OS reservation map is still unknown.

The original SALAA FD base overlaps Android's usual kernel range. It can only be considered in a boot path where LK replaces the Android kernel with this UEFI payload at the agreed address. The scatter file's `boot` value is a flash offset, not proof of a DRAM load address. No evidence yet establishes the entry PC, EL, SP, argument registers, DT pointer, or recovery after a failed handoff.

## Design choices

- Use the **MT6785/RM6785 device evidence** for address maps and target setup. MT6768/Helio G85 code is a useful MediaTek reference, not a drop-in hardware match. The UFS PHY, clock tree, panel, touch, and PMIC registers must not be copied solely because both chips are MediaTek.
- Keep generic UEFI services and standard class drivers in the image by default. They do not claim hardware support without a matching host-controller/board driver. Hardware-specific writes stay out until the register contract is known.
- Keep the bootloader's existing framebuffer and publish its observed stride separately from visible resolution. The panel/DSI remains bootloader-owned; GOP pixel ordering still needs confirmation.
- Reserve `0xF0000000–0xF3FFFFFF` from UEFI allocations for HakiOS's 64 MiB XNUP handoff. The hosted audit checks this interval does not overlap SALAA RAM descriptors. A firmware memory map reservation is not a substitute for testing the BootShim and XNU loader on the phone.
- Preserve upstream Mu-Silicium and HakiOS notices and pin their source commits. Do not commit proprietary phone firmware, extracted vendor DXE drivers, a Darwin image, signing keys, or the raw serial-bearing DTB.

## Repository structure

`source/` is intentionally a **Mu-Silicium workspace overlay**, not a complete standalone EDK II checkout. The hosted workflows copy its contents to the matching Mu-Silicium paths before running the platform build.

```text
Resources/Configs/Salaa.toml                  FD base, size and block count
source/Platforms/Realme/SalaaPkg/              SALAA DSC/DEC/FDF, BDS modules, memory map
source/Silicon/MediaTek/MT6785Pkg/             SALAA SEC/MT6785 platform glue
source/Silicon/Silicium/SiliciumPkg/Drivers/    SALAA SimpleFb override (1088-pixel stride)
integrations/hakios/lock.env                   reviewed upstream repository and commit pins
tools/                                          source/memory/FD integration audits and overlay setup
.github/workflows/edk2-build.yml                vanilla Mu-Silicium hosted compile
.github/workflows/hakios-uefi-build.yml          HakiOS-patched hosted compile + BootShim compile
.github/workflows/source-audit.yml               docs, hashes and source inventory audit
artifacts/release-v0.1.0/                       preserved user-supplied historical build outputs
docs/salaa-device-evidence.md                   sanitized phone and source evidence
```

The `SalaaPkg.fdf` defines a 4 MiB firmware device containing a compact firmware volume and SALAA SEC image. `SalaaPkg.dsc` selects AArch64, the generic EDK II console, BDS, shell, disk, USB class, reset, timer, and runtime modules; `MT6785Pkg.dsc.inc` supplies the core/GIC and library mappings. The SEC hook is deliberately a no-op until the LK entry state and MT6785 early initialization are known.

## Reproducing the hosted builds

### Recommended: GitHub Actions

1. Open **Actions → SALAA EDK II firmware build** for the vanilla Mu-Silicium build or **SALAA HakiOS UEFI integration build** for the patched variant.
2. Select **Run workflow** on `main`, or push a change to the relevant platform/workflow files. The HakiOS workflow is source-pinned, applies the same upstream `hakios.patch`, `files/`, and banner generation as its `apply.sh`, overlays SALAA, compiles the FD, and builds BootShim with the SALAA FD/XNUP addresses.
3. Inspect the uploaded build and log artifacts. A push to `main` that passes the HakiOS integration workflow also creates a `v0.3.0-hakios.<commit>` prerelease containing the generated FD, BootShim, checksum, provenance, and FD verification output. Existing releases are retained.
4. Verify an artifact before use with `sha256sum -c SHA256SUMS` and inspect `verification.json`/`PROVENANCE.md`. These checks establish source/build integrity, not device safety.

The builds use Ubuntu 24.04 GitHub-hosted runners and upstream `setup_env.sh -p apt`. Source revisions are pinned; apt packages and some Mu dependencies are still fetched by upstream setup, so this is a **repeatable source build procedure**, not a bit-for-bit reproducible toolchain lock. Build logs record the SALAA, Mu-Silicium, and HakiOS commit IDs.

### Reproducing the HakiOS source tree and build manually

These commands describe a contributor's reproduction procedure; no EDK II build is run on the development workstation by this project session. `integrations/hakios/lock.env` is authoritative for exact revisions.

```bash
git clone https://github.com/iSciFiLite/rmx2151-salaa-uefi.git salaa-uefi
git clone https://github.com/oosexgang/hakios-uefi.git hakios-uefi
git -C hakios-uefi checkout 687fa6c6a5a70f43184de92980e0a76eea5f2c4b
git clone https://github.com/Project-Silicium/Mu-Silicium.git mu
git -C mu checkout e51cd877e31c8fbb322032dc4cb6715e83d25ad3
git -C mu submodule update --init --recursive
bash salaa-uefi/tools/prepare-hakios-mu.sh "$PWD/hakios-uefi" "$PWD/mu"
cd mu
./setup_env.sh -p apt
python3 build_uefi.py -d Salaa -r DEBUG -c
make -C BootShim REQUIRES_KERNEL_HEADER=1 FD_BASE=0x40000000 \
  FD_SIZE=0x00400000 \
  'CFLAGS=--target=aarch64-linux-gnu -DPAYLOAD_BASE=0xF0000000'
```

The build output is under `mu/Build/SALAA/`; locate `SALAA_EFI.fd` there. Run `python3 salaa-uefi/tools/verify-uefi-fd.py <path-to-SALAA_EFI.fd>` and `python3 salaa-uefi/tools/verify-hakios-integration.py mu`. The last command audits source hooks and handoff constants. A real fastboot image additionally needs the **matching SALAA boot DTB and ramdisk**, correct Android boot-image header parameters, and verified LK load/entry state; those inputs/contracts are not available yet, so the workflow does not emit a boot image.

### Vanilla Mu-Silicium build

The vanilla workflow uses Mu-Silicium commit `4bd8ae83bf37de41777f19e0777db70f59383a6d`, recursively initializes its submodules, copies `source/.` into the workspace, installs upstream dependencies, runs `python3 build_uefi.py -d Salaa -r DEBUG -c`, then verifies that the FD is exactly 4 MiB and has a valid PI firmware-volume signature and length. Run [41260823722](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41260823722) passed on 2026-11-01 for SALAA source commit `22554c432ba6ef7eca59c4733885ff30a56a7686`. Its FD SHA-256 is `4d0bdc13590f925d7558c923413bc4013f1107e3d4828180730594ebf74eb646` and remains in [prerelease `v0.2.0-actions.22554c4`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.2.0-actions.22554c4). This historical image predates the 1088-pixel GOP and XNUP map corrections; use a newer hosted build for those changes.

## Roadmap: what has been done and what remains

> ✅ = done + CI verified. ⚠️ = partial/stub. ❌ = not started.

| Gate | Status | Evidence / remaining gap |
|---|---|---|
| Package / layout | ✅ | Mu overlay, DEC/DSC/FDF, SEC, BDS/Shell — all CI green (Actions: 41304338725) |
| Device evidence | ✅ | Live DT, /proc/iomem, fb0, F.11 OFP scatter/LK header, atag,videolfb analysed |
| Memory map | ✅ | 8 GiB RAM, 28 descriptors, XNUP `0xF0000000` 64 MiB reserved |
| GOP / framebuffer | ✅ | Base `0x75D30000`, size `0x02CD0000`, Stride 1088, 32 bpp, 1080×2400, Tianma NT36672C (atag,videolfb confirmed) |
| HakiOS integration | ✅ | Pinned HakiOS build + BootShim CI pass; release published (Actions: 41304338689) |
| XNU handoff | ⚠️ | XNUP window reserved (0xF0000000); BootShim configured; phone runtime fastboot handoff unverified on hardware |
| **UART SerialPortLib** | ✅ | `MT6785SerialPortLib` at `0x11002000`, 115200-8N1, 26 MHz mode-3 (EDK II compliant) |
| **UFS Block I/O Driver** | ✅ | Hardware-probing driver: probes VER/HCE/HCS/CAP at `0x11270000`; exposes 8 LUNs (4096-byte blocks) |
| USB class stack | ⚠️ | Generic UEFI USB bus/HID/storage class modules present; SoC xHCI host controller driver unported |
| PSCI / secondary CPU | ⚠️ | Standard Arm PSCI interface used; secondary CPU startup unverified on physical hardware |
| DSI / panel init | ⚠️ | LK-owned framebuffer handoff utilized; native panel init not required when bootloader initializes display |
| ACPI / SMBIOS | ⚠️ | SMBIOS strings set for RMX2151/SALAA; ACPI tables not generated (DeviceTree-first boot path) |
| Capsule update | ❌ | Module present; persistent NVRAM / authenticated variable storage not yet enabled |
| Hardware boot test | ❌ | First controlled `fastboot boot` requires user in-person physical test with unlocked bootloader |

### What has been done (cumulative, 2026-10-26 – 2026-11-01)

1. **Package skeleton** — Mu-Silicium workspace overlay, `SalaaPkg.{dec,dsc,fdf}`, SEC stub, GIC/core PCDs from F.11 DTB.
2. **Memory map** — 28-descriptor `MemoryMapLib` with XNUP carve-out at `0xF0000000`.
3. **GOP stride fix** — `PcdFrameBufferPixelsPerScanLine|1088` from live `fb0` evidence.
4. **SimpleFbDxe override** — stride-aware SimpleFb in SALAA overlay.
5. **HakiOS UEFI integration** — pinned workflow, `apply.sh` simulation, BootShim build, v0.3.0-hakios release.
6. **UART SerialPortLib** — `MT6785SerialPortLib` implementing `mediatek,mt6577-uart` at `0x11002000` (8250-compat, MTK high-speed mode 3, 115200-8N1 / 26 MHz). Baud parameters derived from device kernel `8250_mtk.c` and confirmed DTS node.
7. **UFS Block I/O stub** — `MT6785UfsBlockIoDxe` registering 8 LUN handles (HCI `0x11270000`, PHY `0x11fa0000`); reads blocked pending full driver.
8. **Firmware evidence** — F.11 OFP scatter, LK2 header analysis, UART DTS node (GPIO53=RX, GPIO54=TX), UFS DTS node, all documented in `docs/salaa-device-evidence.md`.

### What has NOT been done

- Full UFS host+PHY init (clock enable, UICC link startup, UFSHCIv2 task ring, LUN reads/writes)
- USB host-controller driver (MT6785 SSUSB / xHCI)
- PSCI secondary CPU startup verification on hardware
- DSI / display panel init (bootloader-owned; vendor panel code required)
- Confirmed `smem_start` framebuffer physical base and byte pitch
- LK DRAM load address, entry PC/EL, handoff registers
- SALAA `fastboot boot` attempt
- Verified HakiOS XNU runtime on SALAA hardware

“CI green” does not mean “working device feature.”

## Change history and procedure log

All dates below are UTC and refer to repository/CI work unless explicitly labeled as owner-supplied phone evidence.

| Date | Work and result |
|---|---|
| 2026-10-26 | Recorded the provided EDK II archive, SALAA F.11 input bundle, live DTB, and hashes. The archive was a generic EDK II source tree, not a complete Mu-Silicium SALAA workspace/build log. |
| 2026-11-01 | Added/repaired the SALAA Mu platform overlay, DEC/DSC/FDF, SEC/memory-map sources, hosted build closure, FD checks, and artifact retention. Several failed hosted runs identified and fixed missing package/library/source paths; logs are available in Actions. |
| 2026-11-01 | Hosted vanilla Mu-Silicium build run `41260823722` passed and produced the 4 MiB FD in `v0.2.0-actions.22554c4`. The older `v0.1.0` release files and checksums were preserved. |
| 2026-11-01 | Owner supplied rooted RMX2151 identity, 8 GiB live memory evidence, framebuffer `mtkfb` mode/virtual stride, and repeated malformed `/proc/iomem` line. The raw live DTB remains private to avoid publishing its serial-bearing boot arguments. |
| 2026-11-01 | Added a SALAA GOP stride override, framebuffer bounds check, HakiOS/Mu source pins and overlay-preparation tool, static integration checks, XNUP memory reservation, and a hosted HakiOS FD + AArch64 BootShim workflow. The generated outputs are released only after hosted verification. |
| 2026-11-01 | Extracted F.11 OFP scatter/LK header, stock DTS files from device kernel tree. Documented UART `0x11002000` with GPIO53/54 pins, baud parameters (MTK mode-3, 26 MHz), and UFS HCI/PHY addresses. Implemented `MT6785SerialPortLib` (8250-compat, 115200-8N1) and `MT6785UfsBlockIoDxe` stub. |
| 2026-11-01 | Fixed EDK II SerialPortLib prototypes (SerialPortSetControl/GetControl/SetAttributes) in commit `b7c6ea9`. Enabled development branch push triggers in workflow configurations (`e8c47d9`). Verified complete dual-build pass on GitHub Actions: vanilla Mu build [41304338725](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41304338725) and HakiOS integration [41304338689](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41304338689). Merged into `main`, tagged `v0.4.0-uart-ufs.e8c47d9`, and closed PR #1. |
| 2026-11-02 | Extracted live physical framebuffer parameters from phone `atag,videolfb` (`fb_base=0x75d30000`, `vramSize=0x02cd0000`, Tianma NT36672C, 60.15 Hz). Declared `PcdFrameBufferBase` and `PcdFrameBufferSize` PCDs. Upgraded `MT6785UfsBlockIoDxe` with live MMIO hardware probing (VER, HCE, HCS, CAP, MTK_HW_VER) and media presence detection for all 8 LUNs. |

When a later change is built or device-tested, update this table and the status matrix with the exact date, commit, run/log link, and evidence. Do not convert a build pass into a hardware-support claim.

## Evidence needed from the rooted phone

The framebuffer mode/virtual stride is now known, but the sysfs command did not return `line_length`, `smem_start`, or `smem_len`, and the filtered `dmesg` returned no matching lines. For the GOP contract and MTK UFS path, run these separately in Termux; each command escalates only for that read:

```sh
su -c 'for f in name bits_per_pixel line_length virtual_size smem_start smem_len format; do printf "%s=" "$f"; cat "/sys/class/graphics/fb0/$f" 2>/dev/null; done; find /sys/class/graphics/fb0 -maxdepth 1 -type f -printf "%f\n" 2>/dev/null | sort'
su -c 'dmesg 2>/dev/null | grep -Ei "ufshci|ufs phy|ufs|mtkfb|framebuffer|fb0|td4330|nt36672c" | tail -100'
su -c 'find /dev/block/by-name -maxdepth 1 -type l -printf "%f -> %l\n" 2>/dev/null | grep -Ei "xbl|lk|boot|dtbo|log|ufs"'
```

Please redact serial numbers, device identifiers, and UUIDs. Do not upload a full `xbl`/firmware image publicly. A private extract or a list of UFS-related UEFI driver names/versions from your own XBL can help identify an implementation path; redistribution rights and licensing must be checked before any binary is used.

## Credits

- **[oosexgang/hakios-mt6768](https://github.com/oosexgang/hakios-mt6768)** — HakiOS MT6768/Helio G85 reference device; provided the structural reference for MT6785 UEFI porting strategy, MemoryMapLib pattern, and XNUP handoff design.
- **[Project Silicium / Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium)** — upstream EDK II platform framework; SEC/DXE architecture, SiliciumPkg library classes, and SimpleFbDxe driver basis.
- **[oosexgang/hakios-uefi](https://github.com/oosexgang/hakios-uefi)** — HakiOS UEFI patch stack (BootShim, XnuLoader, XNUP); integrated at pinned commit `687fa6c6a5a70f43184de92980e0a76eea5f2c4b`.
- **[UniXNU](https://github.com/UniXNU)** — XNU port design and HakiOS XNU kernel reference.
- **[elohim-etz/android_kernel_realme_mt6785](https://github.com/elohim-etz/android_kernel_realme_mt6785)** — SALAA kernel source used to verify UART `8250_mtk.c` register layout and `ufshci` driver references.
- **[elohim-etz/android_device_realme_RM6785](https://github.com/elohim-etz/android_device_realme_RM6785)** — Device tree overlay reference for SALAA.

## Benefits of successful XNU kernel compile (HakiOS context)

A successful HakiOS XNU kernel compile on/for SALAA:

1. **Proves source closure** — all XNU subsystems, Mach primitives, BSD layer, IOKit, and the HakiOS device abstraction layer build without error for AArch64 MT6785.
2. **Generates `mach_kernel` / XNU image** — required input for `mkxnuimg.py` to produce the UEFI payload at `0xF0000000`.
3. **Enables UEFI payload testing** — with a compiled XNU image, `fastboot boot` can load the UEFI FD containing BootShim → XnuLoader → XNU without flashing Android.
4. **Validates HakiOS driver stubs** — IOKit kext layer and any MT6785-specific drivers (GPU, UFS, display) must compile against the XNU headers; errors reveal missing symbols or mismatched ABIs.
5. **Unlocks the HakiOS log path** — XnuLoader uses `EFI_BLOCK_IO_PROTOCOL` to find the `logdump` partition; a compiled XNU image + working UFS stub makes this path testable.
6. **Enables Mach boot protocol verification** — boot-args, memory map, DTB pointer, and framebuffer descriptor are passed from BootShim to XNU; a compiled kernel allows checking these at first entry.
7. **Demonstrates AArch64v8.2 toolchain readiness** — the MT6785 is ARMv8.2-A+crypto+rcpc; a clean compile proves the cross-compiler and SDK target the correct ISA.

## License and references

SALAA platform code and imported Mu-Silicium SEC/driver code retain the upstream BSD-2-Clause-Patent notice. Local audit utilities use MIT in [`LICENSE-CODE`](LICENSE-CODE). Keep licenses with all future source imports; do not commit extracted proprietary firmware modules.

- [Project Silicium Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium); pinned [build guide](https://github.com/Project-Silicium/Mu-Silicium/blob/4bd8ae83bf37de41777f19e0777db70f59383a6d/Building.md).
- [HakiOS UEFI](https://github.com/oosexgang/hakios-uefi), its pinned `BASE`, `hakios.patch`, `files/`, and `apply.sh` procedure.
- [HakiOS XNU UEFI boot design](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/docs/uefi-boot.md) and [image packer](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/mkxnuimg.py).
- [Realme SALAA Android device tree](https://github.com/realme-mt6785-devs/android_device_realme_salaa) and the owner-supplied live-device evidence.
