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

# DEVIATION: use a SALAA-owned subset of shared defaults instead of the full upstream DSC include.
# The upstream include injects unrelated components and uses package-root aliases that are wrong
# for this workspace; SalaaPkg.dsc owns the complete component list.
!include SalaaPkg/Include/SalaaMuDefaults.dsc.inc

# The supplied SiliciumNullPkg instance provides the SEC PlatformPeim FV HOB.
# Explicitly select the actual upstream implementation for this target.
[LibraryClasses]
  PlatformPeiLib|Silicon/Silicium/SiliciumNullPkg/Library/PlatformPeiLibNull/PlatformPeiLibNull.inf

# The supplied Mu_Basecore implementation is under ArmPlatformPkg/MemoryInitPei.
# Override the stale SiliciumPkg-local path from the shared include.
[LibraryClasses]
  MemoryInitPeiLib|ArmPlatformPkg/MemoryInitPei/MemoryInitPeiLib.inf

[PcdsFixedAtBuild]
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"MTK   "

[LibraryClasses]
  # Explicit repository-root paths for the supplied Mu package trees.
  MsBootManagerSettingsLib|Common/Mu/PcBdsPkg/Library/BootManagerSettingsDxeLibNull/BootManagerSettingsDxeLibNull.inf
  MsNetworkDependencyLib|Common/Mu/PcBdsPkg/Library/MsNetworkDependencyLib/MsNetworkDependencyLib.inf
  MsAltBootLib|Common/Mu_OEM_Sample/OemPkg/Library/MsAltBootLib/MsAltBootLib.inf
  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  ArmSmcLib|MdePkg/Library/ArmSmcLib/ArmSmcLib.inf

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
  MemoryMapLib|Silicon/MediaTek/MT6785Pkg/Library/MemoryMapLib/MemoryMapLib.inf
  SerialPortLib|Silicon/MediaTek/MT6785Pkg/Library/SerialPortLib/SerialPortLib.inf

# SALAA SEC rule: emit the SEC module as a PE32 section in the firmware volume.
