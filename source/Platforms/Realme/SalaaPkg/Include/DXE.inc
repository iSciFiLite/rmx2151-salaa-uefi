# SALAA EDK II DXE firmware volume contents.
# Hardware-specific MT6785 peripheral drivers are intentionally not borrowed
# from MT6768 or MT6789; only the verified bootloader framebuffer handoff is
# consumed by SimpleFbDxe.
INF MdeModulePkg/Core/Dxe/DxeMain.inf
INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
INF MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
INF EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
INF MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
INF FatPkg/EnhancedFatDxe/Fat.inf
INF MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
INF MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
INF MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
INF ArmPkg/Drivers/ArmGicDxe/ArmGicDxe.inf
INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf
INF SiliciumPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf
INF Silicon/MediaTek/MT6785Pkg/Drivers/UfsBlockIoDxe/UfsBlockIoDxe.inf
INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
INF ShellPkg/Application/Shell/Shell.inf
