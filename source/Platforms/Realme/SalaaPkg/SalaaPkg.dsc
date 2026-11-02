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
  FLASH_DEFINITION               = Platforms/Realme/SalaaPkg/SalaaPkg.fdf
  # DEVIATION: unsigned UEFI Shell bring-up image; Android AVB remains a separate boot chain.
  DEFINE ENABLE_SECUREBOOT       = 0
  USE_CUSTOM_DISPLAY_DRIVER      = 0

!include Silicon/MediaTek/MT6785Pkg/MT6785Pkg.dsc.inc

[PcdsFixedAtBuild]
  # User-provided live RMX2151L1 evidence: /memory base 0x40000000, size 8 GiB.
  # The F.11 base DTB says 0x3E605000, but its live Android tree and RAM map
  # show the 8 GiB configuration. This PCD is only a physical-address bound;
  # MemoryMapLib below exposes the observed non-overlapping System RAM ranges.
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x40000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x200000000
  # The existing 4 MiB FD layout places the stack immediately after it. These
  # addresses are usable only when LK loads SALAA_EFI.fd as the replacement
  # boot payload at 0x40000000; they overlap the stock Android kernel layout.
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x40000000
  gArmTokenSpaceGuid.PcdFdSize|0x00400000
  gArmTokenSpaceGuid.PcdFvBaseAddress|0x40000000
  gArmTokenSpaceGuid.PcdFvSize|0x00400000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"realme"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"RMX2151_11_F.11"
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x40400000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x40000
  # SimpleFbDxe's mode description; this does not initialize the DSI panel or
  # establish the reserved framebuffer address from the LK handoff.
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferWidth|1080
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferHeight|2400
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferColorDepth|32
  # Rooted SALAA fb0 reports virtual_size=1088,9600 for 1080x2400 at 32 bpp.
  gSalaaPkgTokenSpaceGuid.PcdFrameBufferPixelsPerScanLine|1088
  # Live DT atag,videolfb: fb_base=0x75d30000, vramSize=0x02cd0000
  gSalaaPkgTokenSpaceGuid.PcdFrameBufferBase|0x75D30000
  gSalaaPkgTokenSpaceGuid.PcdFrameBufferSize|0x02CD0000
  # Serial SimpleTextInOut must connect without a user-created boot option.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConInConnectOnDemand|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0

  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemManufacturer|"realme"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Realme 7"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"RMX2151"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"RMX2151"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemBoardModel|"SALAA"

[Components]
  Silicon/MediaTek/MT6785Pkg/Sec/PlatformSec.inf
  MdeModulePkg/Core/Dxe/DxeMain.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
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
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  SiliciumPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf
  ArmPkg/Drivers/ArmGicDxe/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  # MT6785 UFS Block I/O stub: publishes EFI_BLOCK_IO_PROTOCOL for 8 LUNs
  # at UFS HCI 0x11270000 / M-PHY 0x11fa0000 (DT-confirmed).
  # ReadBlocks returns EFI_NO_MEDIA until a full host driver is added.
  Silicon/MediaTek/MT6785Pkg/Drivers/UfsBlockIoDxe/UfsBlockIoDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
  ShellPkg/Application/Shell/Shell.inf

[BuildOptions]
GCC:*_*_*_CC_FLAGS = -fno-stack-protector
