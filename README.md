# SALAA UEFI — Realme 7 (RMX2151)

An experimental UEFI **porting record** for the Realme 7, codename SALAA, based on MediaTek MT6785 / Helio G95. This repository contains an early EDK II platform configuration and SEC entry implementation. **It is not a reproducible firmware build or bootable release, and it is not compatible with HakiOS. Do not flash it.**

A separate [Nemo UEFI repository](https://github.com/iSciFiLite/rmx2001-nemo-uefi) tracks Realme 6 (RMX2001) work. It deliberately does not inherit SALAA's memory addresses.

## Status at a glance

| Component | Present in source | Tested on Actions | Verified on RMX2151 | Status |
|---|---:|---:|---:|---|
| SALAA platform DSC/FDF | Yes | Static metadata only | No | Incomplete |
| MT6785 SEC entry / assembly | Yes | No EDK II build | No | Initial source |
| RAM discovery and HOB handoff | Referenced, dependency absent | No | No | Blocked |
| Serial console | Referenced, implementation absent | No | No | Blocked |
| UEFI GOP / panel | No working driver or handoff | No | No | Unsupported |
| UFS Block I/O | No MT6785 UFS DXE driver | No | No | Unsupported |
| PSCI / secondary CPUs | Generic EDK components selected | No | No | Unverified |
| HakiOS boot | No | No | No | **Not compatible yet** |
| Firmware image | `v0.1.0` release FD restored and provenance-pinned | Original local EDK II build succeeded; no hosted firmware build yet | No | Historical artifact; not device-verified |

“Present in source” means files/configuration are checked in. It does not mean the code compiles or runs. The hosted workflow audits the package and its claims; it is not an EDK II firmware build or an emulator/device test.

## HakiOS compatibility

HakiOS's AArch64 EFI loader needs firmware that can launch EFI applications and provide a valid memory map, GOP framebuffer, PSCI CPU startup, and UFS Block I/O for the read-only `xnusys` root. It also requires safe, non-overlapping firmware, stack, loader, kernel, framebuffer, and RAM reservations.

| HakiOS requirement | SALAA firmware status |
|---|---|
| AArch64 UEFI boot services | Not demonstrated by a reproducible image |
| Valid RAM map and memory descriptors | **Not verified.** The SEC implementation looks up a `DXE Heap` region from an absent memory-map library. |
| Safe firmware/FD/stack placement | **Not verified.** `0x40000000` FD base and `0x40400000` stack are source assumptions, not an LK/UEFI handoff contract. |
| GOP framebuffer | **Not provided or verified.** Width/height PCDs do not initialize the DSI panel or create GOP. |
| PSCI secondary CPU start | Not verified |
| UFS Block I/O / read-only `xnusys` | **Not provided.** No MT6785 UFS host or Block I/O DXE driver is included. |
| HakiOS boot/recovery path | Not tested; no image or recovery recipe |

**Answer: SALAA UEFI has not been made HakiOS-compatible.** The shared MT6785 facts in [HakiOS MT6785](https://github.com/iSciFiLite/hakios-mt6785) do not make this firmware satisfy those prerequisites. Nemo has the same compatibility blockers plus its own unverified board handoff.

## Interoperability targets

| Target | Intended interface | Current evidence / gap |
|---|---|---|
| Mu-Silicium / EDK II | Native EDK II platform package, building against pinned Mu-Silicium workspace dependencies | The v0.1.0 image came from a successful original Ubuntu EDK II build. The current public source package is incomplete, so a clean GitHub-hosted rebuild has not succeeded or been established. SALAA is not an upstream-supported Mu-Silicium target. |
| XNU UEFI boot path | Standard AArch64 UEFI application and boot-service contract | No XNU EFI-loader boot has been tested with this firmware; standard UEFI-facing intent is not evidence that the firmware provides all services the loader requires. |
| HakiOS MT6785 | GOP, PSCI, memory handoff, and read-only UFS Block I/O for HakiOS | **Not compatible yet.** GOP, PSCI behavior, safe memory map, and UFS Block I/O are not verified/provided. |

The projects should converge on standard UEFI protocols where possible. Mu-Silicium is the firmware build/dependency ecosystem; XNU and HakiOS are consumers of firmware services. Building under EDK II alone does not establish runtime cross-compatibility with either consumer.

## Hardware and source configuration

The checked-in SALAA files describe:

- Realme 7 RMX2151, MT6785, AArch64, build name `SALAA`.
- Eight CPU cores in two clusters and GICv3 addresses from the referenced F.11 device tree. CPU startup behavior is not demonstrated.
- A 1080×2400 framebuffer mode. This is only a mode description; no panel initialization or framebuffer reservation is established.
- Firmware base `0x40000000`, size 4 MiB, and primary stack base `0x40400000`. These are **unverified source assumptions**. Android's kernel load address and the observed DT memory range do not establish a safe UEFI FD/stack/payload reservation.
- `ENABLE_SECUREBOOT = 0` in the EDK II build configuration. This setting alone does not disable Android Verified Boot.

The Realme 7 has board/display/touch variants. The target must be identified from the exact device revision and its bootloader/device-tree handoff before using board-specific addresses. No stock firmware, firmware dump, key, Darwin image, or device data is included here.

## What is done and what is not

### Done

- Added a SALAA-named EDK II DSC/FDF, MT6785 DSC include, SEC module, entry assembly, and PlatformSecLib source.
- Recorded MT6785 / SALAA facts and the upstream Mu-Silicium / EDK II starting point.
- Completed an EDK II `DEBUG_GCC` build in the original Ubuntu workspace and published the output in release `v0.1.0`.
- Restored the release FD plus SEC EFI/DLL/map outputs under `artifacts/release-v0.1.0/`, with filenames and hashes that distinguish them from future Actions output.
- Added a GitHub Actions source/claim audit. It checks repository hygiene, preserves the release FD checksum, and surfaces unresolved source dependencies.
- Separated Nemo documentation into [its own repository](https://github.com/iSciFiLite/rmx2001-nemo-uefi).

### Not done

- No clean-checkout EDK II firmware build. The current package references files not tracked here, including `SalaaPkg/Include/DXE.inc`, `SalaaPkg/Include/SalaaMuDefaults.dsc.inc`, `MT6785Pkg/Library/MemoryMapLib`, and `MT6785Pkg/Library/SerialPortLib`. `ShellBootDxe` is also referenced but not supplied by this repository.
- No proven SEC input contract, RAM discovery, memory reservations, framebuffer handoff, serial output, or display-panel initialization.
- No UFS controller/PHY initialization or UEFI Block I/O implementation.
- No verification of PSCI, CPU start, firmware boot services, shell, storage, or recovery.
- No HakiOS boot or physical-device validation.

The v0.1.0 release remains available and its generated outputs are restored in this repository under `artifacts/release-v0.1.0/`. These are historical build outputs, not a device-verified firmware release. The current source checkout still lacks dependencies required to reproduce that build. No new Actions-built firmware image is published yet.

### Preserved v0.1.0 outputs

| File | Origin / SHA-256 |
|---|---|
| [`SALAA_EFI.release-v0.1.0.fd`](artifacts/release-v0.1.0/SALAA_EFI.release-v0.1.0.fd) | Published `v0.1.0` release archive; `5e6ab9e0e9376b634e5393b39b9889c671e746a49f3f582c90b858b75bf1ace7` |
| `SalaaSec.release-v0.1.0.efi` | Release archive SEC output; SHA-256 recorded in [`PROVENANCE.md`](artifacts/release-v0.1.0/PROVENANCE.md) |
| `SalaaSec-PlatformSec.release-v0.1.0.efi/.dll/.map` | Release archive module outputs and linker map; hashes recorded in [`PROVENANCE.md`](artifacts/release-v0.1.0/PROVENANCE.md) |

The attached [`salaaPkg-v0.1.0.zip`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/download/v0.1.0/salaaPkg-v0.1.0.zip) remains unchanged. Its SHA-256 is `bfb8dd33d8082a2a3d81bce43e3e6c8e82b4fa900208566a4268e34f0d93dba1`. The FD is also available as a [separate release download](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/download/v0.1.0/SALAA_EFI.release-v0.1.0.fd). An Actions-built output, if produced, will have a separate `actions-<commit>` name and checksum.

## Roadmap

| Gate | Work required | State |
|---|---|---|
| 1. Source closure | Add or pin every package/include/library/module dependency and preserve upstream licenses | **Open** |
| 2. Hosted reproducible build | Build from a clean GitHub Actions runner using pinned dependency revisions; retain logs and temporary artifacts | **Open** |
| 3. SEC and RAM contract | Establish actual LK entry registers, RAM ranges/reservations, FD load address, stack, and HOB map for each RMX2151 revision | **Open — requires device evidence** |
| 4. Basic firmware services | Prove serial, boot manager, shell, runtime services, and stable recovery entry | **Open** |
| 5. GOP | Implement or consume the actual handoff framebuffer; verify format, stride, address, and lifetime | **Open — requires device evidence** |
| 6. PSCI | Verify method, CPU IDs, secondary-core startup, and XNU handoff | **Open — requires device evidence** |
| 7. UFS | Provide and validate UFS Block I/O and read-only GPT/partition access; then boot a test payload | **Open** |
| 8. HakiOS integration | Boot the HakiOS loader/kernel read-only and record UART/serial and framebuffer logs | **Open** |
| 9. Recovery and release | Document a verified recovery route; publish only reproducible artifacts with matching source and provenance | **Open** |

A green source-audit action closes none of gates 2–9. Those gates remain visible until matching build logs or device test evidence exists.

## References

- [Project Silicium / Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium) and its [build guide](https://github.com/Project-Silicium/Mu-Silicium/blob/main/Building.md): EDK II workspace and package-layout reference. SALAA is not an upstream-supported target.
- [HakiOS MT6785](https://github.com/iSciFiLite/hakios-mt6785): current loader/kernel-side contract and SALAA/Nemo status.
- [Realme 7 SALAA device tree](https://github.com/realme-mt6785-devs/android_device_realme_salaa): hardware reference; not copied into this repository.
- [Nemo UEFI status](https://github.com/iSciFiLite/rmx2001-nemo-uefi): separate RMX2001 record.
- [TianoCore EDK II](https://github.com/tianocore/edk2).

## Hosted checks

The [SALAA source audit workflow](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/workflows/source-audit.yml) runs on pushes, pull requests, and manual dispatch. It verifies hashes/sizes for all five preserved release outputs, checks the FD's PI firmware-volume signature/length, and reports unresolved source references. It does **not** compile a firmware image, emulate the handset, or verify device behavior. A reproducible hosted EDK II build remains open because the source dependencies listed above are absent from the published checkout.

## Development and safety

Use the upstream Mu-Silicium build guide only after completing source closure and adding a SALAA `DeviceBuild.py`/workspace configuration with pinned dependencies. Do not infer a usable build command from the incomplete DSC/FDF. Do not flash the historical 4 MiB output or any experimental image. Keep the stock boot, vendor_boot, vbmeta, and recovery images; use a verified recovery route before future device experiments.

Do not enable a hardware feature just because a corresponding component exists for another MediaTek SoC. The G85 MT6768 donor is useful for HakiOS software interfaces and porting patterns; its board registers, eMMC path, GPU power sequence, and peripheral protocols are not a SALAA hardware implementation.

## Credits and licensing

The port uses EDK II / Mu-Silicium structure and retains BSD-2-Clause-Patent SPDX identifiers on SALAA-specific source files. The source-audit helper is MIT-licensed in [`LICENSE-CODE`](LICENSE-CODE). Preserve all original file notices and the licenses of any dependency later added. No stock firmware, proprietary firmware, signing keys, Darwin image, or generated firmware binary is distributed.
