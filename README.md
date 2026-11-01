# SALAA UEFI — Realme 7 / RMX2151

An experimental EDK II / Mu-Silicium firmware port for the Realme 7 SALAA family (MediaTek MT6785, Helio G95). The repository contains a platform package and a hosted build workflow. A successful build would prove that EDK II can produce an FD; it would **not** prove that the FD can safely boot on a handset. Do not flash or chainload an image until the LK entry contract and recovery path are confirmed.

## Current result

| Area | Current implementation | Evidence / limit |
|---|---|---|
| EDK II platform | SALAA DEC/DSC/FDF, Mu `DeviceBuild.py`, pinned Mu-Silicium workflow | First hosted run found a missing platform DEC before C compilation; the package descriptor fix is queued |
| SEC and early memory | AArch64 SEC plus static RAM/MMIO map from the supplied phone evidence | Entry registers and LK load/entry addresses remain unknown |
| RAM size | 8 GiB physical span (`0x40000000` + `0x200000000`) | Live DT and `MemTotal` agree; F.11 base DTB is an older/different 1 GiB declaration |
| GOP | SimpleFbDxe consumes the observed bootloader framebuffer reservation | Does not initialize DSI/panel; format, stride, and lifetime are unverified |
| Serial | Null SerialPortLib | No UART MMIO writes until clock, pins, baud, and handoff are confirmed |
| Storage | Generic disk/partition/FAT and USB class drivers included | No MT6785 UFS host/PHY driver or USB host controller; internal UFS is unsupported |
| Shell | EDK II Shell and BDS included in the firmware volume | Requires successful SEC/DXE startup; not device-tested |
| PSCI/reset | Generic Arm GIC, timer, and PSCI reset components selected | Device SMC behavior and secondary CPU start are untested |
| HakiOS compatibility | Not compatible yet | UFS Block I/O and hardware validation are missing |

The current target identity from Termux is `ro.product.model=RMX2151L1`; the build fingerprint names `RMX2156T2/RMX2156L1`. That variant/build identity should be confirmed before treating this profile as compatible with other SALAA models.

## Work completed

- Added a GitHub Actions build path using Mu-Silicium pinned at [`4bd8ae83`](https://github.com/Project-Silicium/Mu-Silicium/commit/4bd8ae83bf37de41777f19e0777db70f59383a6d), including recursive submodules, hosted dependency installation, EDK II build logs, FD structure checks, and an artifact upload. The first run completed Mu setup but stopped before compilation because the platform DEC was missing; the DEC fix is queued for the next run.
- Reconstructed the missing platform DEC/`DeviceBuild.py`, FD config, DXE firmware-volume list, SEC helper sources, and a SALAA memory-map library. The SEC platform hook avoids undocumented MT6785 register writes.
- Used the supplied live DT plus current Termux memory evidence to correct the physical-memory bound to 8 GiB and define observed Android-visible System RAM ranges. The F.11 base DTB still differs and is not assumed to be the active tree.
- Enabled generic UEFI boot/runtime, BDS/Shell, graphics console/SimpleFb, reset, timer, disk filesystem, and USB class modules where upstream code exists. Generic class drivers do not replace missing SoC host-controller support.
- Preserved the historical `v0.1.0` release files and hashes unchanged. They remain user-reported local-build outputs, not hardware-verified firmware.

## Still incomplete

- Hosted EDK II compilation: the first `SALAA EDK II firmware build` run reached the platform pre-build and stopped on package identification, before C compilation. A package DEC has been added; the next hosted run must confirm the fix and uncover any remaining compile errors.
- SEC entry contract: no LK trace identifies entry PC, exception level, stack pointer, argument registers, DT pointer, or FD load address. `0x40000000` is also the stock Android kernel load region; the FD must replace the kernel payload rather than coexist with it.
- RAM reservations: the current map is reconstructed from Android's `/proc/iomem`; one pasted System RAM interval was inverted and is omitted pending a short filtered recheck. The complete pre-OS LK reservation map is not known.
- Display: the live tree reports a framebuffer reservation at `0x75d30000` (`0x02cd0000` bytes), but SimpleFb's 1080×2400×32-bit assumptions and pixel layout need on-device confirmation. DSI panel initialization is absent.
- Serial: no MT6785 UART driver. The build intentionally uses the null library instead of risking an unclocked UART write.
- UFS: the DT declares HCI at `0x11270000` and PHY at `0x11fa0000`, but DT addresses alone do not supply UFS clocks, PHY sequencing, reset, DMA/IOMMU, or Block I/O. Internal storage is not usable from UEFI yet.
- USB host, touch, power/PMIC, RTC correctness, secure-boot policy, PSCI CPU startup, recovery behavior, XNU/HakiOS handoff, and all physical-device behavior remain unverified or unsupported.

## Device evidence and memory layout

The sanitized input inventory and DT analysis are in [`docs/salaa-device-evidence.md`](docs/salaa-device-evidence.md). The supplied live phone facts are:

- `ro.product.model`: `RMX2151L1`.
- Build fingerprint: `realme/RMX2156T2/RMX2156L1:12/SP1A.210812.016/Q.174ebd4_fa4d:user/release-keys`.
- `MemTotal`: 7,830,224 kB (about 7.46 GiB available to Linux).
- Live DT `/memory`: base `0x40000000`, size `0x200000000` (8 GiB physical span).
- `/proc/iomem` has discontiguous System RAM ending at `0x23fffdfff`; holes are left out of the current map.
- The F.11 base DTB instead declares `0x3e605000` bytes. Its scatter offsets are flash offsets, not RAM load addresses.

The FD base `0x40000000`, size 4 MiB, and stack `0x40400000` are carried forward from the historic SALAA FD layout. `/proc/iomem` shows Android's kernel normally occupies this area. They are therefore usable only in a boot path where LK replaces the Android kernel with this UEFI payload at the expected load/entry address. The current memory map does not establish that contract. Do not use Android's `boot.img` kernel address as proof of an FD load address.

The raw live DTB is intentionally not published because its boot arguments contain a device serial. Input hashes and sanitized facts are documented in the evidence report. No stock firmware, private signing keys, or Darwin image is included.

## HakiOS and other consumers

**SALAA UEFI has not been made compatible with HakiOS, XNU, or another OS.** Generic UEFI modules and a successful EDK II build would provide a firmware-volume image and compile-time integration only. HakiOS additionally needs a validated memory map, working GOP, stable boot-services handoff, and read-only UFS Block I/O. None has been exercised on this handset. Mu-Silicium is the build/dependency base; it is not proof of consumer compatibility.

The enabled SimpleFb path may reuse a bootloader-initialized framebuffer without touching the DSI registers. UFS and USB class drivers are present only as protocol layers: no MT6785 UFS host/PHY or USB host implementation is included. Unsupported hardware stays explicitly unsupported in the status table rather than receiving copied MT6768/MT6789 register assumptions.

## Build on GitHub Actions

The [`SALAA EDK II firmware build`](.github/workflows/edk2-build.yml) workflow checks out Mu-Silicium at the pinned commit, recursively checks out its required submodules, overlays this platform, runs the upstream dependency setup, builds `DEBUG` with `build_uefi.py -d Salaa`, validates the PI firmware-volume signature/size, and uploads the FD, SHA-256, provenance, and logs. Failed runs also retain logs. The workflow does not run on this local Ubuntu machine.

Artifacts are named `SALAA_EFI.actions-<commit>.fd` and retained by Actions for 30 days. They are experimental compile outputs, not flashable releases. The old `v0.1.0` release is preserved separately. The [`source audit`](.github/workflows/source-audit.yml) checks documentation, source inventory, and old artifact provenance; it does not replace the EDK II build.

## Roadmap

| Gate | Required work | State |
|---|---|---|
| Source closure and pinned workspace | Platform build files and upstream SEC helpers are present; first CI error was the missing platform DEC, now fixed | **In progress** |
| Hosted clean build | Build with pinned Mu-Silicium on GitHub Actions, fix errors, retain exact logs and FD hash | **Retry pending** |
| SALAA boot contract | Confirm LK load/entry address, EL, SP, registers/DT pointer, and pre-OS reserved ranges | **Open — needs bootloader evidence** |
| Memory map | Correct the one inverted `/proc/iomem` line and validate against LK/DT reservations | **Open** |
| GOP | Confirm inherited framebuffer base, size, pixel format, stride, and display lifetime | **Open — hardware test** |
| Serial and recovery | Add measured UART support and document a verified recovery path | **Open — hardware evidence** |
| UFS Block I/O | Implement MT6785 HCI/PHY setup and validate read-only GPT/partition access | **Open** |
| PSCI and firmware services | Verify reset, timer, GIC, variables, shell, and secondary CPU behavior | **Open — hardware test** |
| HakiOS/XNU integration | Boot a test loader, collect serial/GOP/memory-map logs, and validate `ExitBootServices` | **Open** |

No step is considered complete from a source audit alone. Only the hosted compile can be verified without the device; memory, GOP, UFS, and boot behavior require real SALAA evidence.

## References and licenses

- [Project Silicium / Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium) and its [pinned build guide](https://github.com/Project-Silicium/Mu-Silicium/blob/4bd8ae83bf37de41777f19e0777db70f59383a6d/Building.md).
- [Realme SALAA Android device tree](https://github.com/realme-mt6785-devs/android_device_realme_salaa) and the supplied live phone evidence.
- [HakiOS MT6785](https://github.com/iSciFiLite/hakios-mt6785) for the intended consumer-side protocol contract.
- SALAA source and imported Mu SEC helper files retain `BSD-2-Clause-Patent` notices; the audit tool remains MIT-licensed in [`LICENSE-CODE`](LICENSE-CODE). Preserve upstream notices for future imports.
