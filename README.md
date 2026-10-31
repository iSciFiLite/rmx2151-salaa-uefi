# Nemo UEFI work has moved

Nemo UEFI tracking now lives in the dedicated repository:

**[iSciFiLite/rmx2001-nemo-uefi](https://github.com/iSciFiLite/rmx2001-nemo-uefi)**

This legacy branch is retained only as a migration pointer. Do not use it as a firmware source or flash any artifact from its history. The branch inherited SALAA build outputs and an incomplete shared MT6785 source snapshot; neither was a Nemo firmware build. New Nemo documentation, status, and future source changes belong in the separate repository.

The separate repo currently contains a device/porting record only. Nemo has no reproducible UEFI build, verified memory map, GOP, UFS Block I/O path, or HakiOS-compatible firmware. Its hosted workflow audits claims and repository contents; it does not build firmware or test hardware.

For SALAA-only work, see [rmx2151-salaa-uefi main](https://github.com/iSciFiLite/rmx2151-salaa-uefi). For HakiOS loader/kernel status, see [hakios-mt6785](https://github.com/iSciFiLite/hakios-mt6785).
