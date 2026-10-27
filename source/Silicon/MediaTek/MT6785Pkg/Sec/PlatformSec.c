#include <Uefi.h>
/**
  SALAA MT6785 SEC entry, adapted from the supplied Silicium SEC implementation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PrePiLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PlatformSecLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <Library/TimerLib.h>
#include <Library/HobLib.h>

#include "Sec.h"

STATIC
VOID
SalaaSecFatal (
  IN CONST CHAR8 *Message
  )
{
  SerialPortWrite ((UINT8 *)Message, AsciiStrLen (Message));
  CpuDeadLoop ();
}

#ifndef MDEPKG_NDEBUG
STATIC
VOID
PrintFirmwareVersion (VOID)
{
  DEBUG ((EFI_D_WARN, "\n"));
  DEBUG ((EFI_D_WARN, "SALAA RMX2151 MT6785\n"));
  DEBUG ((EFI_D_WARN, "Firmware Version %s Build at %a on %a\n",
          PcdGetPtr (PcdFirmwareVersionString), __TIME__, __DATE__));
  DEBUG ((EFI_D_WARN, "\n"));
}
#endif

STATIC
EFI_STATUS
InitializeMemory (VOID)
{
  EFI_STATUS                    Status;
  EFI_MEMORY_REGION_DESCRIPTOR  UefiMemoryRegion;
  EFI_HOB_HANDOFF_INFO_TABLE   *HobList;
  UINT64                        SystemMemoryBase;
  UINT64                        SystemMemorySize;
  UINT64                        FdBase;
  UINT64                        FdSize;
  UINT64                        StackBase;
  UINT64                        StackSize;

  ZeroMem (&UefiMemoryRegion, sizeof (UefiMemoryRegion));
  Status = LocateMemoryRegionByName ("DXE Heap", &UefiMemoryRegion);
  if (EFI_ERROR (Status) || UefiMemoryRegion.Address == 0 || UefiMemoryRegion.Length == 0) {
    return EFI_NOT_FOUND;
  }

  SystemMemoryBase = FixedPcdGet64 (PcdSystemMemoryBase);
  SystemMemorySize = FixedPcdGet64 (PcdSystemMemorySize);
  FdBase           = FixedPcdGet64 (PcdFdBaseAddress);
  FdSize           = FixedPcdGet32 (PcdFdSize);
  StackBase        = FixedPcdGet64 (PcdCPUCoresStackBase);
  StackSize        = FixedPcdGet32 (PcdCPUCorePrimaryStackSize);

  if (((UefiMemoryRegion.Address & EFI_PAGE_MASK) != 0) ||
      ((UefiMemoryRegion.Length & EFI_PAGE_MASK) != 0) ||
      ((FdBase & EFI_PAGE_MASK) != 0) ||
      ((FdSize & EFI_PAGE_MASK) != 0) ||
      ((StackBase & EFI_PAGE_MASK) != 0) ||
      ((StackSize & EFI_PAGE_MASK) != 0) ||
      (SystemMemorySize == 0) ||
      (SystemMemoryBase > MAX_UINT64 - SystemMemorySize) ||
      (UefiMemoryRegion.Address < SystemMemoryBase) ||
      (UefiMemoryRegion.Length > SystemMemorySize) ||
      ((UefiMemoryRegion.Address - SystemMemoryBase) >
       (SystemMemorySize - UefiMemoryRegion.Length)) ||
      (FdBase < SystemMemoryBase) ||
      (FdSize == 0) ||
      (FdSize > SystemMemorySize) ||
      ((FdBase - SystemMemoryBase) > (SystemMemorySize - FdSize)) ||
      (StackBase < SystemMemoryBase) ||
      (StackSize == 0) ||
      (StackSize > SystemMemorySize) ||
      ((StackBase - SystemMemoryBase) > (SystemMemorySize - StackSize))) {
    return EFI_COMPROMISED_DATA;
  }

  if (((FdBase < UefiMemoryRegion.Address + UefiMemoryRegion.Length) &&
       (UefiMemoryRegion.Address < FdBase + FdSize)) ||
      ((StackBase < UefiMemoryRegion.Address + UefiMemoryRegion.Length) &&
       (UefiMemoryRegion.Address < StackBase + StackSize)) ||
      ((FdBase < StackBase + StackSize) &&
       (StackBase < FdBase + FdSize))) {
    return EFI_COMPROMISED_DATA;
  }

  HobList = HobConstructor (
              (VOID *)UefiMemoryRegion.Address,
              UefiMemoryRegion.Length,
              (VOID *)UefiMemoryRegion.Address,
              (VOID *)(UefiMemoryRegion.Address + UefiMemoryRegion.Length)
              );
  if (HobList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PrePeiSetHobList (HobList);
  Status = MemoryPeim (UefiMemoryRegion.Address, UefiMemoryRegion.Length);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BuildStackHob (
    FixedPcdGet64 (PcdCPUCoresStackBase),
    FixedPcdGet32 (PcdCPUCorePrimaryStackSize)
    );
  return EFI_SUCCESS;
}

STATIC
VOID
SecMain (VOID)
{
  EFI_STATUS Status;

  PlatformInitialize ();

#ifndef MDEPKG_NDEBUG
  PrintFirmwareVersion ();
  InitializeDebugAgent (DEBUG_AGENT_INIT_POSTMEM_SEC, NULL, NULL);
  SaveAndSetDebugTimerInterrupt (TRUE);
#endif

  Status = InitializeMemory ();
  if (EFI_ERROR (Status)) {
    SalaaSecFatal ("SALAA SEC: memory initialization failed\n");
  }

  BuildCpuHob (ArmGetPhysicalAddressBits (), PcdGet8 (PcdPrePiCpuIoSize));
  SetBootMode (BOOT_WITH_DEFAULT_SETTINGS);

  Status = PlatformPeim ();
  if (EFI_ERROR (Status)) {
    SalaaSecFatal ("SALAA SEC: PlatformPeim failed\n");
  }

  ProcessLibraryConstructorList ();

  Status = DecompressFirstFv ();
  if (EFI_ERROR (Status)) {
    SalaaSecFatal ("SALAA SEC: FV decompression failed\n");
  }

  Status = LoadDxeCoreFromFv (NULL, 0);
  if (EFI_ERROR (Status)) {
    SalaaSecFatal ("SALAA SEC: DXE core load failed\n");
  }

  SalaaSecFatal ("SALAA SEC: DXE handoff returned\n");
}

VOID
SecEntry (VOID)
{
  ArmDisableDataCache ();
  ArmInvalidateInstructionCache ();
  ArmEnableInstructionCache ();

  InvalidateDataCacheRange (
    (VOID *)FixedPcdGet64 (PcdCPUCoresStackBase),
    FixedPcdGet32 (PcdCPUCorePrimaryStackSize)
    );
  InvalidateDataCacheRange (
    (VOID *)FixedPcdGet64 (PcdFdBaseAddress),
    FixedPcdGet32 (PcdFdSize)
    );

  ASSERT (((UINTN)SecVectorTable & ARM_VECTOR_TABLE_ALIGNMENT) == 0);
  ArmWriteVBar ((UINTN)SecVectorTable);
  SecMain ();
}
