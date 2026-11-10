# SALAA UEFI — Realme 7 / RMX2151 / MT6785

> [!NOTE]
> **Inclusive Development Active**: This component is developed in lockstep with the HakiOS kernel and XnuLoader in the unified inclusive repository: [**iSciFiLite/hakios-rm6785-salaa**](https://github.com/iSciFiLite/hakios-rm6785-salaa). All cross-layer enhancements, shared memory map definitions, and unified fastboot boot image packaging are authored atomically there. Standalone repositories are synchronized at release milestones via `tools/split-subtrees.sh`.

**Status updated 2026-11-02.** This repository contains a SALAA EDK II platform overlay for [Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium), plus a pinned [HakiOS UEFI](https://github.com/oosexgang/hakios-uefi) integration and a hosted combined HakiOS/XNU Android-v2 boot image. CI can verify the build and container; only a phone test can verify LK acceptance, UEFI entry, the XNU handoff, or peripherals.

> **Do not flash these images.** The combined image is intended only for a temporary `fastboot boot` RAM test from an external USB host, after the owner has a known recovery route and an unlocked bootloader. It does not write the boot or LK partitions. Its LK acceptance and hardware behavior are unverified. The FD base overlaps Android's normal kernel load area, so it must replace the stock kernel payload exactly as packaged.

## Support and verification status

| Area | Implemented now | Evidence and remaining limit |
|---|---|---|
| SALAA platform | AArch64 SEC/DXE, board package, memory descriptors, SimpleFb GOP, standard EDK II BDS/Shell | The vanilla Mu-Silicium source compiled on hosted Actions; not hardware-tested on phone. |
| Mu-Silicium | Pinned workspace overlays and reproducible package configuration | Vanilla pin `4bd8ae83bf37de41777f19e0777db70f59383a6d` passed hosted build run [41260823722](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41260823722). HakiOS uses its own base pin, below. |
| HakiOS UEFI | Upstream BootShim/XNUP and BootManager/XnuLoader patch stack; SALAA overlay; separate hosted build and BootShim job | Build result is recorded in the latest [HakiOS integration Actions run](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/workflows/hakios-uefi-build.yml). A successful compile is source integration only. It does not make the phone runtime-compatible. |
| HakiOS/XNU fastboot handoff | SALAA MT6785 kernel config and UEFI profile; hosted XNU build and XNUP packaging | The combined image uses the matching HakiOS FD and BootShim plus the pinned MT6785 kernel in HakiOS's XNUP format. Hosted builds do not establish phone compatibility. |
| Vanilla UEFI diagnostic boot image | **Produced and hosted-verified** as [v0.7.0-bootimg.41512150665](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.0-bootimg.41512150665) | 1,951,744-byte Android v2 image, SHA-256 `59a416bb6e99884ee1401f0cd20d3e5440bfc31a6e8fcc72eef169a4241b75d3`. Combines gzip(BootShim + vanilla SALAA FD) with matching F.11 header/ramdisk/DTB. Diagnostic only; it is not the intended HakiOS/XNU image. |
| SALAA HakiOS/XNU boot image | **Unified image released; phone test pending** | [v1.0.0-unified](https://github.com/iSciFiLite/hakios-rm6785-salaa/releases/tag/v1.0.0-unified) (7,467,008-byte Android v2 image, SHA-256 `888d795fc66ae30da7753646785cac17623c4cbc1b3c077307d6da0583c5a319`). Combines gzip(BootShim + SALAA FD + XNUP payload with 7 native MT6785 drivers + open hello ramdisk) with verified F.11 ramdisk and DTB. `fastboot boot` only; no phone test yet. |
| GOP/framebuffer | Uses live-DT display reservation and reports 1080×2400, 32 bpp, **1088 pixels per scan line** | `fb0` reports `mtkfb`, 32 bpp, virtual size `1088,9600`; owner-supplied Linux pstore independently logs PA `0x75d30000–0x789fffff`, matching the known 46,989,312-byte allocation. Actual byte pitch, pixel channel order, scanout, and lifetime across UEFI handoff still need confirmation. The DSI/panel is not initialized by this GOP driver. |
| Internal UFS | Read-only UFSHCI register probe at `0x11270000`; optional eight Block I/O placeholders report no media | Android pstore confirms Linux MTK UFSHCI at this address negotiates gear 3 and enumerates Samsung `KM8V8001JM-B813`, direct-access LUNs 0/1. Probe now decodes standard HCS bits: DP bit 0, UTRLRDY bit 1, UTMRLRDY bit 2, UCRDY bit 3. UEFI still does not initialize the PHY/link, discover LUN capacity, or transfer blocks; `ReadBlocks` returns `EFI_NO_MEDIA`. |
| UART/serial | MT6785 SerialPortLib configures `0x11002000`, MTK mode 3, 115200-8N1 from an assumed 26 MHz clock | Hosted compilation only. UART output, pinmux, clock state, and boot handoff are not hardware-verified. |
| USB | Generic USB bus, keyboard, and mass-storage class modules are included | MT6785 SSUSB/xHCI host driver and role/power setup are absent. USB peripherals are not expected to work yet; this is not a blocker for the current `fastboot boot` smoke test. |
| CPU, GIC, timer, reset | Generic Arm/GIC code and MT6785 core/PSCI declarations are included | PSCI uses the declared SMC conduit; timer calibration, secondary CPU start, and reset behavior require on-device testing after UEFI starts. |
| DSI / panel | No native panel driver; GOP uses the framebuffer initialized by LK | Native DSI init is intentionally not part of the first boot path. Verify the LK framebuffer handoff; add a panel driver only if that handoff is unavailable or insufficient. |
| ACPI / SMBIOS | SMBIOS identity PCDs are configured; firmware volume accepts optional ACPI tables | No SALAA ACPI tables are generated. Current HakiOS path is DeviceTree-first, so ACPI is not a boot gate. SMBIOS runtime publication is not phone-verified. |
| Capsule / persistent variables | Generic capsule/runtime-variable modules are present in the build | No persistent variable store or authenticated update path is configured. This is deferred and not needed for one-shot RAM boot; do not use capsule update until it is designed and tested. |

**Compatibility statement:** SALAA UEFI source and the HakiOS MT6785/SALAA XNU configuration compile in hosted CI, and a combined v0.7.1 HakiOS/XNU smoke image is published. Optional UFS Block I/O placeholders report no media; `ReadBlocks` returns `EFI_NO_MEDIA` and exposes no usable storage. FD, UART, GOP, HakiOS loader, and XNU execution have not been verified together on SALAA hardware. The v0.7.0 image remains a separate vanilla UEFI diagnostic. The v0.7.1 payload contains only the open `hello` ramdisk, not Apple/Darwin userland, and does not establish hardware compatibility.

HakiOS documentation distinguishes a loader started from a UEFI volume from its `fastboot boot` shim/payload route. The upstream packer documents `gzip(BootShim + UEFI FD + optional XNUP) + DTB`; this SALAA repo adapts that carrier to the phone's v2 boot header. The companion [`hakios-mt6785`](https://github.com/iSciFiLite/hakios-mt6785) repo already contains a SALAA kernel configuration and had a successful hosted kernel compile; its earlier workflow did not publish the output or combine it into XNUP. The new workflow does that using the open `hello` ramdisk, with UFS root disabled because the SALAA UEFI's UFS protocol currently returns no media. The Redwood/Qualcomm FD and XNUP are not used. This is a kernel/loader smoke path, not a full Darwin userland image or a verified phone boot. See [HakiOS UEFI](https://github.com/oosexgang/hakios-uefi), [HakiOS XNU UEFI boot notes](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/docs/uefi-boot.md), and [the XNU image packer](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/mkxnuimg.py).

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

The build output is under `mu/Build/SALAA/`; locate `SALAA_EFI.fd` there. Run `python3 salaa-uefi/tools/verify-uefi-fd.py <path-to-SALAA_EFI.fd>` and `python3 salaa-uefi/tools/verify-hakios-integration.py mu`. The last command audits source hooks and handoff constants. HakiOS's upstream `mkxnuimg.py` defaults to Redwood's 2 MiB FD, `0x9FD00000`, and header v1, so it is not used unchanged. The SALAA output has passed hosted image verification; actual SALAA LK acceptance is unknown. Use `fastboot boot SALAA_UEFI_BOOT.img` only for a temporary RAM test; do not flash it to `boot` or modify either LK partition.

### Build a one-shot SALAA Android boot image

The **SALAA one-shot UEFI boot image** GitHub Actions workflow creates an Android v2 `boot.img` intended to start UEFI. It preserves the F.11 boot header, ramdisk, and separate DTB, replaces the Linux kernel with `gzip(BootShim + vanilla SALAA FD)`, then verifies the header, addresses, embedded bytes, ramdisk, DTB, and 32 MiB size limit. This contains no XNUP/XNU payload; it is for testing entry into the vanilla UEFI shell. A successful packaging audit is not proof that SALAA LK accepts or boots it.

1. The verified 32 MiB F.11 `boot.img` is kept as an asset in a draft-only release, not in Git or a public release. The workflow downloads it with the repository token and checks its size and SHA-256 (`27227c7345e412998ec7394dd37646adc5da8614f71d5653f747840fcdcc57c5`). Do not upload the 10.5 GB OFP.
2. In **Actions → SALAA one-shot UEFI boot image**, select **Run workflow** on `main`.
3. Download the image and verification/provenance files from the run artifact or its prerelease. Verify them with `sha256sum -c SHA256SUMS`.
4. With an unlocked bootloader, the first trial is `fastboot boot SALAA_UEFI_BOOT.img`. This is a temporary RAM boot. Do not use `fastboot flash boot` or flash LK/LK2. No phone boot test has been done; recovery and bootloader acceptance must be confirmed by the owner in person.

The stock input remains outside Git and is accessible only to repository collaborators while its release is a draft. The workflow pins and hash-checks both image tools and verifies the input boot hash. The generated image includes the F.11 ramdisk and DTB; provenance records its source and marks phone boot untested. Because the resulting image carries vendor firmware components, its release is explicitly experimental and must not be mistaken for a general-purpose flash image.

### Build the intended HakiOS + MT6785 XNU image

Run **Actions → SALAA HakiOS XNU fastboot image** on `main`. The hosted Ubuntu jobs check out the pinned companion source at `f50d914`, install the x86_64 Theos Linux toolchain from release `test-210562a` (archive SHA-256 `a72a7a577e2fbe2838b6b5e9c72034fa7d114af96f0e1d4b016f18730ce4056e`) and iPhoneOS 16.5 SDK snapshot at `theos/sdks` commit `0222fd5413cf4b9af096f37b4621afa2688572f7`, build the SALAA XNU kernel and open `hello` ramdisk, and create `SALAA_XNUP.bin` with HakiOS's own `xnuloader/mkpayload.py`. The second job combines it with the matching HakiOS SALAA FD and BootShim from `v0.3.0-hakios.8e6cf0d`, plus the verified F.11 v2 boot wrapper, ramdisk, and DTB. It records input hashes, verification metadata, and toolchain archive hash. The output is a HakiOS/XNU smoke-test image, not a full Darwin installation: its cmdline disables UFS root and it contains no Apple userland. Build and package checks do not prove runtime compatibility or phone boot. Use `fastboot boot SALAA_HAKIOS_XNU_BOOT.img` only; do not flash it.

Hosted run [41520513335](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41520513335) passed both jobs: it built the pinned SALAA kernel and XNUP, assembled the full combined smoke-test image, validated header/load addresses/payload/ramdisk/DTB/size/checksums, and published prerelease [`v0.7.1-hakios-xnu.41520513335`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.1-hakios-xnu.41520513335). The image is 7,464,960 bytes (SHA-256 `b4bf5a1081cd2155a21c0345a22921bbe815e561f9c0fd1e6632d9d5cff611d3`); XNUP SHA-256 is `d8d37e0f895825b8c3186431e3c725be83851fe2533e7d1ab260766697539b41`. It includes kernel, `hello` ramdisk, the pinned toolchain checksum, verification, and provenance. I downloaded the complete release and verified every asset against `SHA256SUMS`. This resolves the prior release-step failure. These are hosted build/package results only; no SALAA phone boot has been performed.

**Payload distinction:** XNUP is HakiOS's named-entry container format, not an XNU project or kernel. This route packages the SALAA/MT6785 kernel built from the pinned `hakios-mt6785` source and the open `hello` ramdisk in the format consumed by HakiOS BootShim/XnuLoader. A kernel from another XNU project can be placed in XNUP only if its architecture, expected entry names, boot arguments, and HakiOS loader/kernel handoff match; the container alone does not make it compatible. UniXNU's `DarwinBoot`/UEFI integration is a separate boot stack and is not being substituted into this HakiOS-first route. UniXNU is not inherently incompatible with HakiOS at the XNU-source level, but its SALAA/MT6785 kernel and loader integration have not been established here. The selected path is Mu-Silicium SALAA firmware + HakiOS BootShim + HakiOS XnuLoader + the pinned MT6785 HakiOS kernel in XNUP.

### Vanilla Mu-Silicium build

The vanilla workflow uses Mu-Silicium commit `4bd8ae83bf37de41777f19e0777db70f59383a6d`, recursively initializes its submodules, copies `source/.` into the workspace, installs upstream dependencies, runs `python3 build_uefi.py -d Salaa -r DEBUG -c`, then verifies that the FD is exactly 4 MiB and has a valid PI firmware-volume signature and length. Run [41260823722](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41260823722) passed on 2026-11-01 for SALAA source commit `22554c432ba6ef7eca59c4733885ff30a56a7686`. Its FD SHA-256 is `4d0bdc13590f925d7558c923413bc4013f1107e3d4828180730594ebf74eb646` and remains in [prerelease `v0.2.0-actions.22554c4`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.2.0-actions.22554c4). This historical image predates the 1088-pixel GOP and XNUP map corrections; use a newer hosted build for those changes.

## Roadmap: what has been done and what remains

> ✅ = done + CI verified. ⚠️ = partial/stub. ❌ = not started.

| Gate | Status | Evidence / remaining gap |
|---|---|---|
| Package / layout | ✅ | Mu overlay, DEC/DSC/FDF, SEC, BDS/Shell — all CI green (Actions: 41304338725) |
| Device evidence | ✅ | Live DT, /proc/iomem, fb0, F.11 scatter, boot.img v2 header/ramdisk/DTB, LK/LK2 hashes, atag,videolfb analyzed |
| Memory map | ✅ | 8 GiB RAM, 28 descriptors, XNUP `0xF0000000` 64 MiB reserved |
| GOP / framebuffer | ✅ | Base `0x75D30000`, size `0x02CD0000`, Stride 1088, 32 bpp, 1080×2400, Tianma NT36672C (atag,videolfb confirmed) |
| HakiOS integration | ✅ | Pinned HakiOS build + BootShim CI pass; release published (Actions: 41304338689) |
| SALAA UEFI-entry Android boot image | ✅ | Hosted package verification passed (run 41512150665); prerelease contains image, SHA256SUMS, verification, and provenance. Physical `fastboot boot` remains untested. |
| SALAA HakiOS/XNU boot image | ⚠️ | Hosted end-to-end build/package passed in run `41520513335`; complete release `v0.7.1-hakios-xnu.41520513335` and all checksums verified. Phone boot remains untested. |
| XNU handoff | ⚠️ | HakiOS XNUP created with the pinned MT6785 kernel, matching BootShim/loader and `0xF0000000` reserved window. Container and boot-image bytes verified in hosted CI; actual loader-to-kernel handoff has not been observed on-device. |
| **UART SerialPortLib** | ⚠️ | EDK II source compiles; `0x11002000`, 115200-8N1 / assumed 26 MHz; physical output not captured |
| **UFS Block I/O** | ⚠️ | Register probe and no-media placeholders compile; PHY/link initialization, LUN discovery, and block transfers are not implemented. |
| USB class stack | ⚠️ | Generic UEFI USB bus/HID/storage modules present; MT6785 SSUSB/xHCI host controller and board role/power setup are absent. |
| PSCI / secondary CPU | ⚠️ | SMC PSCI path is declared; CPU start, timer, and reset behavior require phone testing. |
| GOP / DSI panel | ⚠️ | GOP uses LK's framebuffer; verify its survival, pixel format, and pitch on-device. Native DSI init is conditional, not a first-boot prerequisite. |
| ACPI / SMBIOS | ⚠️ | SMBIOS identity PCDs are configured; runtime publication is unverified. ACPI is omitted for the current DeviceTree-first path and is not a boot blocker. |
| Capsule update | ⚠️ | Deferred: no persistent variable backend or authenticated update flow; not required for a RAM boot. |
| Hardware boot test | ❌ | Next gate: one controlled `fastboot boot` from an external host, known recovery route, and tester logs. No image has been booted on the phone. |

### What has been done (cumulative, 2026-10-26 – 2026-11-02)

1. **Package skeleton** — Mu-Silicium workspace overlay, `SalaaPkg.{dec,dsc,fdf}`, SEC stub, GIC/core PCDs from F.11 DTB.
2. **Memory map** — 28-descriptor `MemoryMapLib` with XNUP carve-out at `0xF0000000`.
3. **GOP stride fix** — `PcdFrameBufferPixelsPerScanLine|1088` from live `fb0` evidence.
4. **SimpleFbDxe override** — stride-aware SimpleFb in SALAA overlay.
5. **HakiOS UEFI integration** — pinned workflow, `apply.sh` simulation, BootShim build, v0.3.0-hakios release.
6. **UART SerialPortLib** — `MT6785SerialPortLib` implementing `mediatek,mt6577-uart` at `0x11002000` (8250-compat, MTK high-speed mode 3, 115200-8N1 / 26 MHz). Baud parameters derived from device kernel `8250_mtk.c` and confirmed DTS node.
7. **UFS Block I/O stub** — `MT6785UfsBlockIoDxe` samples controller registers and registers no-media placeholders; no LUN discovery or data transfers.
8. **Firmware evidence** — F.11 OFP scatter, boot.img v2 and boot-chain files, UART DTS node (GPIO53=RX, GPIO54=TX), UFS DTS node, all documented in `docs/salaa-device-evidence.md`.
9. **Hosted vanilla release** — current 4 MiB SALAA FD built and released as `v0.6.1-vanilla.fd5785f`; checksum and provenance were validated in Actions.
10. **One-shot boot-image packer** — SALAA v2 packer and Actions workflow use the verified F.11 image from a draft-only release, pinned image tools, matching F.11 ramdisk/DTB, and input hash checks.
11. **HakiOS/XNU combined image** — hosted workflow compiled the SALAA MT6785 HakiOS XNU kernel, created XNUP with HakiOS's payload builder, combined it with the matching HakiOS FD/BootShim and F.11 Android-v2 wrapper, and published [v0.7.1-hakios-xnu.41520513335](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.1-hakios-xnu.41520513335). Both jobs passed in [Actions run 41520513335](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41520513335); release checksums verified. It is a `ufs=0 ufs_root=0` smoke payload, not a full Darwin installation or hardware-validated boot.

### What has NOT been done or verified

- **UFS:** only register probing/no-media protocol placeholders exist. Clock/PHY setup, link startup, transfer management, LUN discovery, and real reads need implementation. Required for storage-backed OS boot, not this UFS-disabled smoke image.
- **USB:** no MT6785 SSUSB/xHCI host driver or board role/power setup. Generic class modules alone cannot make USB peripherals work; not required to submit the first fastboot boot request.
- **PSCI and timer:** interface/configuration exists; SMC calls, secondary CPU start, timer, and reset behavior are untested until firmware runs on the handset.
- **Display:** actual framebuffer physical base, byte pitch, pixel order, and lifetime across handoff need verification. Native DSI init is only needed if LK's preinitialized framebuffer cannot be used.
- **LK and memory handoff:** exact load address, entry PC/EL/registers and pre-OS reserved ranges are not confirmed. Android `/proc/iomem` still includes an invalid interval; no address is guessed.
- **Full Darwin/iOS environment:** absent. The released payload has only the open `hello` ramdisk, disables UFS root, and has no Apple userland or provisioned root volume.
- **Capsule/security:** persistent variable storage, authenticated update flow, and production Secure Boot policy are deferred; they are not prerequisites for one-shot RAM boot.
- **Physical validation:** no SALAA `fastboot boot`, UEFI shell observation, XnuLoader/kernel output, UFS read, USB peripheral test, or recovery test has been reported.

“CI green” does not mean “working device feature.”

### Simple priority order

1. Verify release checksums and make one controlled `fastboot boot` attempt from an external host, only with an unlocked bootloader and known recovery route. No partition flashing is required.
2. Send the complete fastboot host log and screen/result. This determines LK acceptance, UEFI entry, GOP handoff, and whether XnuLoader reaches XNU.
3. If startup stops before visible output, a tester with UART access can capture serial logs; the owner does not need to open the handset. After entry is established, prioritize UFS if storage-backed HakiOS is desired; add xHCI only if USB peripherals are a requirement. Keep capsule update and ACPI deferred unless a concrete need arises.

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
| 2026-11-02 | Extracted live physical framebuffer parameters from phone `atag,videolfb` (`fb_base=0x75d30000`, `vramSize=0x02cd0000`, Tianma NT36672C, 60.15 Hz). Declared `PcdFrameBufferBase` and `PcdFrameBufferSize` PCDs. Added UFSHCI register sampling (VER, HCE, HCS, CAP, MTK_HW_VER). The HCS bit interpretation in later documentation was found incorrect and is corrected in the 2026-11-02 source milestone below: DP is bit 0; UCRDY is bit 3. |
| 2026-11-02 | Re-audited the 108,172-byte F.11 sidecar (ZIP metadata/DTB/DTBOs), then found the 10.55 GB `RMX2151_F11.ofp` and its existing selective `selected_f11` extraction; no full extraction was run. Verified the 33,554,432-byte `boot.img` hash against the manifest and parsed header v2, 2048-byte pages, kernel `0x40080000`, ramdisk `0x47c80000`, and DTB/tags `0x4bc80000`. `lk.img`/`lk2.img` exist and are byte-identical; neither needs modification for a RAM boot. Hosted EDK run [41506437166](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41506437166) produced a vanilla FD; follow-up [41507829182](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41507829182) released the 4 MiB FD (SHA-256 `0cef01b241c42ae97dc2776e77e8bbe9431a7ad031eaaeed78f0036c59407c49`) at [`v0.6.1-vanilla.fd5785f`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.6.1-vanilla.fd5785f). F.11 boot input is staged in a draft-only release, not Git. Commit `e437578` produced the vanilla diagnostic Android v2 image in Actions [41512150665](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41512150665); it passed header, load-address, payload, ramdisk/DTB, checksum, and size checks and is published at [`v0.7.0-bootimg.41512150665`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.0-bootimg.41512150665) (1,951,744 bytes; SHA-256 `59a416bb6e99884ee1401f0cd20d3e5440bfc31a6e8fcc72eef169a4241b75d3`). This image uses vanilla FD and has no XNUP; it is a diagnostic, not the HakiOS/XNU target. The companion MT6785 tree has a SALAA XNU configuration and a prior successful hosted compile [41252760580](https://github.com/iSciFiLite/hakios-mt6785/actions/runs/41252760580), but that workflow did not publish a kernel artifact. Its ignored local prebuilt has a different SHA from the recorded CI output and is not reused. Added a SALAA-specific workflow using pinned HakiOS MT6785 source, HakiOS's own XNUP builder, a pinned Theos Linux toolchain/SDK, the HakiOS SALAA FD+BootShim, and the private F.11 boot input. The first hosted compile/package and release evidence are in the following milestone row; full Darwin userland and device boot remain unverified. |
| 2026-11-02 (hosted HakiOS/XNU milestone) | Run [41516039804](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41516039804) produced an initial image whose provenance used the vanilla command. Run [41517036914](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41517036914) rebuilt with corrected image naming and a pinned Theos Linux toolchain/SDK; its kernel and package validation passed, but the release step missed the toolchain checksum asset because of a shell continuation typo. The sidecar was attached, the workflow corrected, and the clean end-to-end rerun [41520513335](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41520513335) passed both jobs and published [`v0.7.1-hakios-xnu.41520513335`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.7.1-hakios-xnu.41520513335). The 7,464,960-byte image SHA-256 is `b4bf5a1081cd2155a21c0345a22921bbe815e561f9c0fd1e6632d9d5cff611d3`; XNUP SHA-256 is `d8d37e0f895825b8c3186431e3c725be83851fe2533e7d1ab260766697539b41`. The release's complete checksum list validated after download. Earlier releases and the prior candidate remain preserved. This completes hosted packaging, not physical boot validation. |
| 2026-11-02 (owner-supplied Termux evidence) | Ran the three read-only queries recorded in `docs/salaa-device-evidence.md`: framebuffer sysfs attributes, filtered `dmesg`, and `/dev/block/by-name` links. Results: `mtkfb`, 32 bpp, virtual size `1088,9600`; no values for line length/framebuffer address/length/format; no matching `dmesg` lines; aliases `dtbo→sdc45`, `lk2→sdc42`, `boot_para→sdc28`, `lk→sdc41`, `logo→sdc44`, `boot→sdc43`. These observations do not verify GOP output or UFS operation. |
| 2026-11-02 (owner attachment follow-up) | Reviewed `commands-info.txt` without committing it because pstore includes a serial-bearing boot command line. The supplied results cover the previously requested `getprop` loop, `/sys/class/scsi_host/host*` metadata, and `/sys/fs/pstore/*`. They report Android properties `flash.locked=1`, `vbmeta.device_state=locked`, verified state green; host `ufshcd`; Linux framebuffer allocation `0x75d30000–0x789fffff`; and MTK UFS link at `0x11270000`, gear 3, Samsung `KM8V8001JM-B813`, direct-access LUNs 0/1. These are Linux/hardware references, not UEFI functionality. The owner-reported Android 16 QPR2 ROM and attached Android 12 fingerprint disagree; fastboot host state is still pending. Next requested host commands: `fastboot devices`, `fastboot getvar product`, `fastboot getvar unlocked`, `fastboot getvar secure`; no boot attempt until unlock and recovery are confirmed. |
| 2026-11-02 (GOP/UFS probe correction) | Used the pstore range to make GOP report the complete reserved allocation (`0x02cd0000`) while retaining visible mode `1080x2400` and stride `1088`; added a GOP range log. Corrected UFSHCI HCS decoding from the previous mistaken note: `DP=bit0`, `UTRLRDY=bit1`, `UTMRLRDY=bit2`, `UCRDY=bit3`, and added read-only decoded state logging. No UFS reset, PHY changes, or transfers are attempted; the driver remains probe-only pending device evidence and a real host driver. |
| 2026-11-02 (hosted verification) | Source commit `297af64` passed hosted vanilla EDK II builds on development [41528575811](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41528575811) and main [41528575950](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41528575950). First HakiOS overlay runs [41528576075](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41528576075) (development) and [41528576633](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41528576633) (main) failed only because the integration audit expected the old GOP size expression. Updated that audit in `7a0511f` to require full reserved-size propagation; source audit [41529278353](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41529278353) and HakiOS EDK II + BootShim integration [41529274597](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41529274597) then passed. The HakiOS FD/BootShim and provenance are retained as Actions artifact `SALAA-HakiOS-UEFI-7a0511f8a0efafb4e741f42b6faa5dc0330e818c` (30-day retention), not a release; no phone test occurred. |
| 2026-11-02 (input-bundle fingerprint audit) | Read `metadata/build.prop` from the supplied SALAA F.11 archive. It gives stock model `RMX2151`, build `RMX2151_11_F.11`, Android 12, and fingerprint `realme/RMX2151/RMX2151L1:12/SP1A.210812.016/Q.bf75e7-1:user/release-keys`. This static firmware identity was already in our inputs and does not need to be requested from the owner; it is distinct from the live/custom-ROM property. |

When a later change is built or device-tested, update this table and the status matrix with the exact date, commit, run/log link, and evidence. Do not convert a build pass into a hardware-support claim.

## Evidence needed from the rooted phone
The matching F.11 stock `boot.img` was present in the local selective extraction and verified against the manifest. It is staged as a draft-only GitHub release asset for the hosted packaging workflow; the 10.5 GB OFP was not uploaded. The custom Android 16 QPR2 ROM is fine for read-only device checks; a dump of its active `boot` partition is optional for comparing current ROM layout. The generated image is for `fastboot boot`, not flashing LK or the `boot` partition.

The owner supplied the framebuffer, kernel-log, and partition-alias queries on
2026-11-02. Results: `mtkfb`, 32 bpp, virtual size `1088,9600`; the requested
line length/base/size/format sysfs files returned no values; the selected
`dmesg` filter returned no lines; and Android listed `dtbo`, `lk2`,
`boot_para`, `lk`, `logo`, and `boot` symlinks (full command/output in
[`docs/salaa-device-evidence.md`](docs/salaa-device-evidence.md)). This does
not establish GOP scanout correctness, UFS LUN mapping, or LK's UEFI handoff.

The owner then supplied `commands-info.txt`. Its sanitized findings and
limits are recorded in [`docs/salaa-device-evidence.md`](docs/salaa-device-evidence.md):
Linux pstore corroborates framebuffer physical range `0x75d30000–0x789fffff`
and shows Android's MTK UFS driver at `0x11270000` negotiating gear 3 and
enumerating Samsung `KM8V8001JM-B813` direct-access LUNs 0 and 1. This is
useful hardware reference evidence, not UEFI GOP or UFS validation. The
attachment's properties report a Realme Android 12 fingerprint and locked /
green verified-boot values, while the owner describes the current ROM as
Android 16 QPR2 Pixel-based. Treat the ROM identity and lock state as
unresolved until confirmed by the external fastboot host. The raw attachment
is not committed because pstore contains a serial-bearing boot command line.

### Next: read-only fastboot host preflight

No more Termux output is needed for the checks already present in
`commands-info.txt`. From a separate computer with Android platform-tools,
connect the phone in bootloader/fastboot mode and run these read-only queries;
paste the output here before any boot attempt:

```sh
fastboot devices
fastboot getvar product 2>&1
fastboot getvar unlocked 2>&1
fastboot getvar secure 2>&1
```

Only consider the temporary `fastboot boot` procedure below if the bootloader
reports unlocked and you have a known recovery route. Do not flash any
partition. Do not upload a full `xbl`/firmware image publicly; private
firmware inspection must be limited to needed driver metadata and checked
for redistribution rights.

### First combined-image test and what it can prove

The release contains a **complete combined boot container for this smoke test**: the F.11 Android-v2 header, stock F.11 ramdisk and DTB, and a compressed kernel payload containing HakiOS BootShim + the matching 4 MiB SALAA HakiOS FD + XNUP (MT6785 XNU kernel + open `hello` ramdisk). It is not a complete Darwin/iOS installation: there is no Apple userland, UFS root is disabled, and the UEFI UFS Block I/O driver currently reports no media.

No partition flashing is required or requested for this test. `fastboot boot` is a temporary RAM-boot request to the phone's current LK from a separate computer/USB host running Android platform-tools; the target phone's Termux cannot serve as its own host while in bootloader mode. Proceed only if the bootloader is unlocked and you already know how to return to your current ROM/recovery. Do **not** run `fastboot flash`, patch LK/LK2, or wipe partitions. A black screen or rejected image is possible; stop and preserve the exact output rather than trying alternate flash commands.

On the external host, download the release assets and run:

```sh
sha256sum -c SHA256SUMS
fastboot devices
fastboot getvar product 2>&1
fastboot boot SALAA_HAKIOS_XNU_BOOT.img 2>&1 | tee salaa-fastboot-boot.log
```

Send the complete `salaa-fastboot-boot.log` and report whether the screen showed the UEFI shell, XnuLoader, kernel output/panic, or stayed black; include a photo if useful. If the phone returns to Android, collect this read-only post-boot evidence in Termux and redact identifiers:

```sh
su -c 'getprop ro.product.model; getprop ro.build.fingerprint; cat /proc/meminfo | head -5; grep "System RAM$" /proc/iomem; ls -la /sys/fs/pstore 2>/dev/null; dmesg 2>/dev/null | grep -Ei "ufs|ufshci|framebuffer|mtkfb|panic|watchdog" | tail -100'
```

If the image reaches UEFI but not XNU, the next useful evidence is a UART serial capture or an XnuLoader screen/log. UART requires an accessible serial adapter/test point; do not open the handset for this project. If `fastboot boot` is rejected or there is no safe recovery route, do not retry by flashing: provide the host output and exact current firmware/ROM fingerprint first.

### What is still source work versus phone-only validation

The combined image and source can be built, inspected, and corrected without touching the phone. The following are **not complete in this release** and do not become complete merely because the image compiles: real UFS link/PHY initialization and block reads; UART output; GOP pixel order/pitch/lifetime; PSCI secondary-CPU start; USB host-controller support; XNU boot arguments and loader handoff; full Darwin userland/root storage; and reset/recovery behavior. Some driver implementation can proceed from public MT6785 sources, but clocks, PHY sequencing, memory reservations, and runtime behavior require device logs/testing. LK's exact FD entry/register contract and the malformed `/proc/iomem` interval also need better evidence; the current Android view alone cannot prove pre-OS reservations. A controlled RAM boot is the next gating experiment, not a flash.

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
5. **Enables later loader testing** — after functional UFS and SALAA boot packaging exist; current Block I/O handles cannot read storage or enable the log path.
6. **Enables Mach boot protocol verification** — compilation permits later testing boot arguments, memory map, DTB, and framebuffer handoff; it verifies none of them by itself.
7. **Demonstrates AArch64v8.2 toolchain readiness** — the MT6785 is ARMv8.2-A+crypto+rcpc; a clean compile proves the cross-compiler and SDK target the correct ISA.

## License and references

SALAA platform code and imported Mu-Silicium SEC/driver code retain the upstream BSD-2-Clause-Patent notice. Local audit utilities use MIT in [`LICENSE-CODE`](LICENSE-CODE). Keep licenses with all future source imports; do not commit extracted proprietary firmware modules.

- [Project Silicium Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium); pinned [build guide](https://github.com/Project-Silicium/Mu-Silicium/blob/4bd8ae83bf37de41777f19e0777db70f59383a6d/Building.md).
- [HakiOS UEFI](https://github.com/oosexgang/hakios-uefi), its pinned `BASE`, `hakios.patch`, `files/`, and `apply.sh` procedure.
- [HakiOS XNU UEFI boot design](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/docs/uefi-boot.md) and [image packer](https://github.com/oosexgang/hakios-xnu-kernel/blob/main/mkxnuimg.py).
- [Realme SALAA Android device tree](https://github.com/realme-mt6785-devs/android_device_realme_salaa) and the owner-supplied live-device evidence.
- [HakiOS MT6768 donor/reference](https://github.com/oosexgang/hakios-mt6768) — credited as the original MediaTek/HakiOS starting point; its SoC-specific code is not treated as an MT6785 driver.

## 2026-11-02 checkpoint verification

- SALAA `main` and `development` point to `04909b672e704f1405e068b2b5749d6ce774473d`; companion `hakios-mt6785` is at `f50d914084733cb9d302beaf39fa824a8d2fe41f`.
- Firmware, HakiOS integration, and source audit runs [41378781111](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41378781111), [41378781123](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41378781123), and [41378781125](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41378781125) succeeded. They verify compile/source/FD structure, not phone operation.
- At this earlier checkpoint, XNU kernel compilation, SALAA-specific boot-image packaging, EFI loader execution, `ExitBootServices`, functional UFS reads, and physical boot remained open. The XNU build and packaging are now completed by the hosted milestone above; loader execution, `ExitBootServices`, functional UFS reads, and physical boot remain unverified.
- Tag `v0.5.0-final.04909b6` is preserved; its prerelease distribution contains hosted checkpoint firmware artifacts. Earlier releases remain intact.
- Follow-up source correction removed fake UFS capacity and zero-filled-success behavior; the accompanying claim that HCS[0] is UCRDY was incorrect. The bit mapping is corrected in the later 2026-11-02 milestone: `HCS_DP=bit0`, `HCS_UCRDY=bit3`. Block I/O placeholders still report no media; this remains a probe stub, not a UFS driver.
- The correction is committed as `562f974` and passed source audit [41501437823](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41501437823) plus HakiOS FD/BootShim hosted build [41501437829](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41501437829) on `development`, then main build [41502349705](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41502349705). The resulting FD is in [release `v0.3.0-hakios.562f974`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.3.0-hakios.562f974); its original `SHA256SUMS` paths were workflow-relative, so use the additional `SHA256SUMS.download` asset for checks after downloading. The checksum workflow has been corrected for future runs.
- On 2026-11-02, current integration commit `8e6cf0d` completed the hosted Mu-Silicium EDK II FD build, BootShim build, image/source checks, and release publication in [Actions run 41503246403](https://github.com/iSciFiLite/rmx2151-salaa-uefi/actions/runs/41503246403). The runner stalled in post-job checkout cleanup, so the run was canceled after all build, verification, artifact-upload, and release steps had succeeded. [Release `v0.3.0-hakios.8e6cf0d`](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/tag/v0.3.0-hakios.8e6cf0d) contains the matching 4 MiB FD, AArch64 BootShim, verification, provenance, and `SHA256SUMS`; both binary hashes were downloaded and verified. Source audits for follow-up commit `756757d` passed on development and main (runs `41503807701` and `41504093654`).
- The 2026-11-02 attachment named `SALAA_F11_mainline_inputs.tar` is ZIP-formatted and byte-identical to the previously recorded F.11 bundle (SHA-256 `43af30c7dcf0ba0413c00bf000c3b2890e8a0325b285713a7e182b844292cf33`); no second extraction was needed. The live DTB hash is unchanged.
