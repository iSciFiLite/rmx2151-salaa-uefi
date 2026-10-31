# Realme 6 Nemo UEFI bring-up

This `nemo` branch starts a device-specific UEFI port for the Realme 6 (RMX2001, Nemo) by reusing
the MT6785 SEC/SoC sources shared with the [SALAA UEFI port](https://github.com/iSciFiLite/rmx2151-salaa-uefi/tree/main).

**This branch is not a buildable firmware or a bootable HakiOS path.** It intentionally does not
copy SALAA's fixed firmware/RAM/stack addresses: the exact Nemo LK handoff, RAM reservations,
framebuffer address, and firmware load window have not been validated. See
[`source/Platforms/Realme/NemoPkg/PORTING.md`](source/Platforms/Realme/NemoPkg/PORTING.md) and
[`nemo-hardware.conf`](nemo-hardware.conf).

## Shared MT6785 facts

- Eight CPU affinities: six A55 cores and two A76-class cores; PSCI CPU start must be verified.
- GICv3: distributor `0x0c000000`, redistributor `0x0c040000`.
- UFSHCI base `0x11270000`; no Nemo-ready UFS Block I/O DXE driver is included.
- Mali-G76 MC4; no UEFI display driver or validated GOP contract is included.

## HakiOS prerequisites

HakiOS's EFI loader needs a working GOP, PSCI for secondary CPUs, and UFS Block I/O with a
read-only `xnusys` partition. Nemo's firmware port must additionally establish the exact memory
map and a safe load/stack/FV reservation before a device image can be built or tested. Current Linux
references do not prove those UEFI services exist.

The main HakiOS profiles and remote source checks are in
[iSciFiLite/hakios-mt6785](https://github.com/iSciFiLite/hakios-mt6785). A separate firmware build
and physical-device test are still required; do not flash this branch.

## Hardware references

- [Nemo mainline device tree](https://github.com/Hari-Pi/realme6-pmos-port/blob/main/linux-realme-nemo/mt6785-realme-nemo.dts)
- [Nemo Linux device/kernel port](https://github.com/Hari-Pi/realme6-pmos-port)
- [MT6785 kernel repository](https://github.com/iSciFiLite/android_kernel_realme_nemo)
- [SALAA UEFI source and shared MT6785 SEC code](https://github.com/iSciFiLite/rmx2151-salaa-uefi/tree/main)

No firmware image, proprietary firmware, stock device tree, or generated binary is distributed.
