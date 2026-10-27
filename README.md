# rmx2151-salaa-uefi
UEFI firmware port for the Realme 7 (RMX2151), powered by MediaTek Helio G95 (MT6785), based on Project Silicium's Mu-Silicium EDK II platform infrastructure.

# SALAA UEFI — Realme 7 (RMX2151)

An experimental UEFI firmware port for the Realme 7, adapted from Project Silicium's Mu-Silicium POCO M5 `stonePkg` reference platform for the MediaTek MT6785 SoC.

> **Project identity:** SALAA UEFI is the porting project; `SalaaPkg` is its platform package; `SALAA_EFI.fd` is the generated firmware image.

## Contents

- [Overview](#overview)
- [Project details](#project-details)
- [Reference platform and porting](#reference-platform-and-porting)
- [Project status](#project-status)
- [Firmware architecture](#firmware-architecture)
- [Platform configuration](#platform-configuration)
- [Repository layout](#repository-layout)
- [Building](#building)
- [Build output](#build-output)
- [Source-package completeness](#source-package-completeness)
- [Distribution recommendations](#distribution-recommendations)
- [Credits and licensing](#credits-and-licensing)
- [Disclaimer](#disclaimer)

## Overview

SALAA is a device-specific UEFI porting project for the Realme 7 (RMX2151). It uses the POCO M5 `stonePkg` platform package from [Project Silicium's Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium) as its reference platform and adapts the platform configuration and firmware integration for the Realme 7 and its MediaTek MT6785 SoC.

The port is built within the Mu-Silicium/EDK II firmware development ecosystem. It is not a POCO M5 firmware image copied unchanged: the reference and target devices use different SoCs, and device-specific configuration and initialization must reflect the Realme 7 hardware.

This repository describes the port, its source package, and the build artifact. A generated firmware image is not a substitute for the source and dependency information required to reproduce the build.

## Project details

| Property | Details |
|---|---|
| Project | SALAA UEFI |
| Target device | Realme 7 (RMX2151) |
| Reference platform | POCO M5 (`stonePkg`) |
| Target SoC | MediaTek Helio G95 (MT6785) |
| Architecture | AArch64 / ARM64 |
| Firmware framework | Mu-Silicium / EDK II |
| Build target | `DEBUG` |
| Toolchain tag | `GCC` |
| Build configuration directory | `DEBUG_GCC` |
| Firmware output | `SALAA_EFI.fd` |
| Firmware image size | 4 MiB (4,194,304 bytes) |

## Reference platform and porting

The POCO M5 `stonePkg` provides the reference platform-package structure for this port. SALAA adapts that starting point to the Realme 7 rather than treating the reference package as a drop-in solution.

| Property | Reference platform | SALAA target |
|---|---|---|
| Device | POCO M5 | Realme 7 |
| Platform package | `stonePkg` | `SalaaPkg` |
| SoC | MediaTek MT6789 | MediaTek MT6785 |
| CPU architecture | AArch64 | AArch64 |

The platform package is one part of the larger firmware workspace. Shared Mu-Silicium/EDK II components, libraries, build tools, and other dependencies may be supplied elsewhere in the workspace or through submodules.

## Project status

The existing Mu-Silicium/EDK II workspace completed the SALAA build and generated the 4 MiB `SALAA_EFI.fd` image. The recorded build log dated October 26, 2026, reports successful firmware-volume generation and build completion.

- [x] Adapted the reference platform-package approach for SALAA.
- [x] Added Realme 7 platform DSC/FDF configuration.
- [x] Integrated MT6785 SEC module configuration and AArch64 entry-point support.
- [x] Configured platform PCDs, libraries, and firmware-volume components.
- [x] Completed an EDK II build in the existing workspace.
- [x] Generated `SALAA_EFI.fd`.
- [ ] Documented a clean-checkout build that is independently reproducible from the published repository.
- [ ] Published complete runtime validation evidence for all UEFI stages, drivers, and device peripherals.

A successful build establishes that the configuration compiled and firmware volumes were generated in the original environment. Runtime behavior depends on the target boot chain, memory state, hardware initialization, and the actual availability and operation of configured drivers. Do not infer support for every peripheral or boot stage solely from build success.

## Firmware architecture

### Platform package: `SalaaPkg`

The platform package contains the device-specific EDK II configuration used to assemble the firmware.

- `SalaaPkg.dsc` — platform description. Selects components, library instances, build options, and platform configuration.
- `SalaaPkg.fdf` — firmware device and firmware-volume description. Specifies how firmware volumes and modules are laid out in the generated image.
- Platform include files — shared configuration and component definitions referenced by the platform metadata.

A platform package is not itself a complete UEFI implementation or a compiled firmware image. It is a set of configuration and source inputs used by the build system.

### Silicon package: MediaTek MT6785

The silicon package contains SoC-specific support and the SEC integration used by the SALAA platform. The supplied source tree includes MT6785 package configuration and a platform SEC module.

The SEC module is part of the early firmware startup path. The implementation integrates platform initialization, memory/HOB preparation, firmware-volume processing, and the handoff toward DXE using EDK II/Mu-Silicium library classes and platform code.

The exact behavior of these steps depends on the selected library implementations, target hardware state, and the bootloader's handoff contract.

### Firmware-volume components

The platform configuration selects the DXE core and drivers/services for areas including CPU/GIC initialization, runtime services, serial console, device paths, console handling, boot-device selection, and UEFI Shell functionality.

Component inclusion in the build configuration indicates intended firmware composition. It does not independently demonstrate that each driver or peripheral has been tested on the physical Realme 7.

## Platform configuration

The supplied platform configuration records the following values:

| Setting | Value |
|---|---|
| Platform name | `SALAA` |
| Platform GUID | `0BDC7D8A-C915-4CDA-BB1D-78D7E1C1FD1E` |
| Architecture | AArch64 |
| Build target | `DEBUG` |
| Toolchain tag | `GCC` |
| Firmware base | `0x40000000` |
| Firmware size | `0x00400000` (4 MiB) |
| Configured system-memory base | `0x40000000` |
| Firmware vendor | `realme` |
| Firmware version string | `RMX2151_11_F.11` |
| UEFI Secure Boot build setting | `ENABLE_SECUREBOOT = 0` |

These are values from the supplied configuration, not a universal payload-loading contract. Firmware integration must respect the actual loader behavior, entry address, memory reservations, and handoff state. A configured firmware base should not be assumed safe for every boot path without considering the target memory map.

UEFI Secure Boot and Android Verified Boot are separate mechanisms. The `ENABLE_SECUREBOOT = 0` build setting does not, by itself, disable or replace Android Verified Boot.

## Repository layout

The logical source layout separates the Realme platform package from the MediaTek silicon package:

```text
source/
├── Platforms/
│   └── Realme/
│       └── SalaaPkg/
│           ├── SalaaPkg.dsc
│           └── SalaaPkg.fdf
└── Silicon/
    └── MediaTek/
        └── MT6785Pkg/
            ├── MT6785Pkg.dsc.inc
            ├── Sec/
            │   └── PlatformSec/
            │       ├── PlatformSec.c
            │       ├── PlatformSec.inf
            │       └── AArch64/
            └── Library/
                └── PlatformSecLib/
```

This is a logical overview, not a complete inventory of every file. Actual source files and paths must match the references in the DSC, FDF, INF, DEC, and include files.

The generated `Build/SALAA/DEBUG_GCC/` tree is a build-output directory, not the source package itself. It may contain architecture-level and module-level outputs, linker artifacts, firmware volumes, and other intermediate products. Identical files at different paths may be retained if the build or packaging workflow expects both locations.

## Building

SALAA was built in an existing Mu-Silicium/EDK II workspace using the AArch64 GCC toolchain with the `DEBUG_GCC` configuration.

A reproducible build requires:

1. The appropriate Mu-Silicium repository revision and its required submodules.
2. The build dependencies and toolchain documented by the upstream project.
3. The SALAA platform package and MT6785 silicon source at their expected workspace paths.
4. All source files, library implementations, metadata, and include files referenced by the platform configuration.
5. The Mu-Silicium build workflow configured to select SALAA.

Consult the upstream documentation for workspace setup and build commands:

- [Project Silicium / Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium)
- [Mu-Silicium building guide](https://github.com/Project-Silicium/Mu-Silicium/blob/main/Building.md)
- [TianoCore EDK II](https://github.com/tianocore/edk2)

The original successful build proves that the build inputs were available in that workspace. Before claiming the public repository is independently reproducible, perform a clean-checkout build using the published source, dependency revisions, and instructions.

## Build output

The recorded firmware image is:

```text
Build/SALAA/DEBUG_GCC/FV/SALAA_EFI.fd
```

- Size: 4,194,304 bytes (4 MiB).
- Architecture: AArch64.
- Build target: `DEBUG`.
- Toolchain tag: `GCC`.

`DEBUG_GCC` combines the EDK II Debug build target and GCC toolchain tag. It does not guarantee that UART output or every debugging facility is enabled or accessible on the target device.

`SALAA_EFI.fd` is the generated firmware artifact. It is not a complete source package, a stock Realme firmware package, or a complete replacement for the Android boot chain.

## Source-package completeness

A build can succeed even when an exported archive omits source files: the original workspace may provide those files through other packages, include paths, submodules, or shared dependencies.

For a reproducible source release, include the SALAA-specific source and metadata and document the external dependencies and their revisions. Preserve the relative paths expected by the build metadata. Do not remove build outputs merely because they look duplicated unless their role in the workflow is understood.

## Distribution recommendations

For a source-oriented GitHub repository, include:

- SALAA platform and MT6785 silicon source files.
- Required DSC, FDF, INF, DEC, and include files.
- Build instructions and the exact dependency/revision information.
- Applicable copyright and license notices.

The final firmware image may be published as a GitHub Release asset. Generated build artifacts, including module outputs and linker maps, may be retained when useful for debugging or reproducing the original build. Source and release artifacts should be clearly distinguished.

## Credits and licensing

SALAA builds on the work and infrastructure of:

- **Project Silicium / Mu-Silicium** — firmware framework and POCO M5 `stonePkg` reference platform.
- **TianoCore EDK II** — UEFI firmware development framework and build system.

The SALAA-specific source files identify the BSD-2-Clause-Patent license. Preserve applicable upstream copyright and license notices, and verify each dependency's licensing terms before redistributing a combined source package or firmware image.

- [Project Silicium / Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium)
- [TianoCore EDK II](https://github.com/tianocore/edk2)

## Disclaimer

SALAA is an experimental, community-developed firmware port and is not an official Realme or Xiaomi product. Device-specific integration and hardware support require appropriate validation. Use experimental firmware only if you understand the target device's boot and recovery procedures.
