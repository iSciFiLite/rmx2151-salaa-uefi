##
#  Copyright (c) 2011 - 2022, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2020, Intel Corporation. All rights reserved.
#  Copyright (c) Microsoft Corporation.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
##

[Defines]
  USE_PHYSICAL_TIMER                         = 1

!include SiliciumPkg/SiliciumPkg.dsc.inc

[PcdsFixedAtBuild]
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"MTK   "

[BuildOptions]
  *_*_AARCH64_CC_FLAGS = -march=armv8.2-a+crypto+rcpc

[PcdsFixedAtBuild]
  # F.11 boot.img and LK-embedded DTB: Cortex-A55 nodes cpu@000..cpu@003
  # and cpu@100..cpu@101; Cortex-A75 nodes cpu@102..cpu@103.
  # Their MPIDR reg values describe eight CPUs across two clusters.
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2
  # F.11 DTB /interrupt-controller reg: GICD 0x0C000000/0x40000;
  # GICR 0x0C040000/0x200000.
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x0C000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x0C040000
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"MediaTek MT6785"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosProcessorPartNumber|"MT6785"

[LibraryClasses]
  PlatformSecLib|SalaaPkg/Library/PlatformSecLib/PlatformSecLib.inf
  MemoryMapLib|SalaaPkg/Library/MemoryMapLib/MemoryMapLib.inf
  # No UART driver is enabled until the RMX2151 UART clock/pin and handoff
  # have been verified. Keep debug output fail-silent rather than MMIO-guess.
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf

# SALAA SEC rule: emit the SEC module as a PE32 section in the firmware volume.
