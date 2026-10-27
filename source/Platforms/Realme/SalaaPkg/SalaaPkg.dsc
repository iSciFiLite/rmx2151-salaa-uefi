##
# SALAA / Realme 7 RMX2151 / MediaTek MT6785.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
[Defines]
  PLATFORM_NAME                  = SALAA
  PLATFORM_GUID                  = 0BDC7D8A-C915-4CDA-BB1D-78D7E1C1FD1E
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SALAA
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = RELEASE|DEBUG
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = SalaaPkg/SalaaPkg.fdf
  # DEVIATION: unsigned UEFI Shell bring-up image; Android AVB remains a separate boot chain.
  DEFINE ENABLE_SECUREBOOT       = 0
  USE_CUSTOM_DISPLAY_DRIVER      = 0

!include Silicon/MediaTek/MT6785Pkg/MT6785Pkg.dsc.inc

[PcdsFixedAtBuild]
  # F.11 boot.img and LK embedded DTB /memory reg: base 0x40000000, size 0x3E605000.
  # F.11 scatter: LK=0x56500000, LK2=0x56700000, boot=0x56900000,
  # DTBO=0x59100000 (flash offsets, not DRAM load addresses).
  # ASSUMPTION: F.11 does not define a UEFI payload load contract or reserve
  # an FD/stack region. boot.img's 0x40080000 kernel address is for Android's
  # kernel and cannot establish where a separately loaded UEFI FD will execute.
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x40000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x3E605000
  # ASSUMPTION: use a 4 MiB FD window at 0x40000000 to fit the 2 MiB DXE FV
  # nested inside the outer FV plus the SEC module. F.11 defines no UEFI
  # payload load contract or reservation; this is a source-layout choice.
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x40000000
  gArmTokenSpaceGuid.PcdFdSize|0x00400000
  gArmTokenSpaceGuid.PcdFvBaseAddress|0x40000000
  gArmTokenSpaceGuid.PcdFvSize|0x00400000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"realme"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"RMX2151_11_F.11"
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x40400000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x40000
  # Serial SimpleTextInOut must connect without a user-created boot option.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConInConnectOnDemand|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0

  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemManufacturer|"realme"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Realme 7"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"RMX2151"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"RMX2151"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemBoardModel|"SALAA"

[Components]
# DEVIATION: ResetSystemRuntimeDxe omitted for first bring-up; Mu_Basecore
# version consumes HwResetSystemLib which has no implementation in the
# attached tree. Restore after Shell boots by pointing MdeModulePkg
# resolution to edk2 instead of Mu_Basecore.
  Silicon/MediaTek/MT6785Pkg/Sec/PlatformSec.inf
  MdeModulePkg/Core/Dxe/DxeMain.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  Silicon/MediaTek/MT6785Pkg/Drivers/ShellBootDxe/ShellBootDxe.inf
  ArmPkg/Drivers/ArmGicDxe/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
  ShellPkg/Application/Shell/Shell.inf

[BuildOptions]
GCC:*_*_*_CC_FLAGS = -fno-stack-protector
