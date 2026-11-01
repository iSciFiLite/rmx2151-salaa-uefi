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
  PlatformSecLib|Silicon/MediaTek/MT6785Pkg/Library/PlatformSecLib/PlatformSecLib.inf
  MemoryMapLib|Platforms/Realme/SalaaPkg/Library/MemoryMapLib/MemoryMapLib.inf
  # MT6785 UART0 SerialPortLib — MMIO implementation at 0x11002000.
  # Compatible: mediatek,mt6577-uart (DT-confirmed, stock-salaa-1.dts).
  # Configured at 115200-8N1 using the 26 MHz SoC reference clock and
  # MTK high-speed mode 3 (HIGHS=3, DLL=1, SAMPLE_CNT=225, SAMPLE_PT=111).
  # This replaces the null stub added in f691125; the register layout and
  # baud parameters are sourced from the device kernel (8250_mtk.c) and the
  # confirmed DT node, NOT guessed from a different SoC.
  SerialPortLib|Silicon/MediaTek/MT6785Pkg/Library/SerialPortLib/SerialPortLib.inf
  # Standard EDK II Shell application libraries (matches ShellPkg's DSC).
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
  OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

# SALAA SEC rule: emit the SEC module as a PE32 section in the firmware volume.
