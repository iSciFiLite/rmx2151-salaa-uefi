# NemoPkg porting gate

The shared MT6785 SEC code in this repository is a starting point, not a Nemo firmware. A Nemo
`NemoPkg.dsc`/`.fdf` is intentionally withheld until the RMX2001 boot-chain address map is known.
Copying SALAA's `PcdSystemMemorySize`, FD base, stack base, or framebuffer reservation could
overwrite live memory on Nemo.

Before adding a buildable platform descriptor, record and review:

1. The exact supported RMX2001 hardware/firmware variant and LK handoff contract.
2. DRAM banks and reserved regions from the matching device tree, including the framebuffer.
3. The firmware image load address, size, relocation behavior, SEC stack, DXE heap, and kernel
   payload reservation, with no overlaps.
4. PSCI CPU affinity and CPU_ON behavior on Nemo.
5. A UFSHCI initialization/handoff that provides UEFI Block I/O; the host base alone is not a
   driver.
6. A working panel GOP or documented headless mode, plus a way to launch the HakiOS EFI app.

The [mainline Nemo device tree](https://github.com/Hari-Pi/realme6-pmos-port/blob/main/linux-realme-nemo/mt6785-realme-nemo.dts)
is a hardware reference. Its bootable-Linux status does not establish a UEFI firmware contract.
Until the requirements above are met, this profile is source documentation only and must not be
built into or flashed as a phone image.
