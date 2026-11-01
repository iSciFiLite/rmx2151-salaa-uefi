/**
  SALAA RMX2151L1 static memory map from the attached live DT and Termux
  /proc/iomem evidence. System RAM holes are left unmapped; they include
  firmware, modem, TEE, display, and other reserved allocations. The active
  Android kernel code/data ranges are not carried over because this payload
  must replace the Android kernel, not coexist with it.

  The FD/stack/heap placement remains a boot-chain assumption. In particular,
  0x40000000 is also the stock Android kernel load region. Do not flash or
  chainload this image until the LK load address and entry contract are proven.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/MemoryMapLib.h>

#define SYSTEM_RAM(Name, Start, EndInclusive) \
  { Name, Start, (EndInclusive) - (Start) + 1, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN }

#define MMIO_REGION(Name, Start, Size) \
  { Name, Start, Size, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE }

STATIC EFI_MEMORY_REGION_DESCRIPTOR mMemoryMap[] = {
  // Same 4 MiB FD and 256 KiB stack windows used by the historical SALAA FD.
  { "UEFI FD",       0x40000000ULL, 0x00400000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, BsCode, WRITE_BACK },
  { "UEFI Stack",    0x40400000ULL, 0x00040000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN },
  // First live System RAM bank ends at 0x4CDFFFFF; reserve one contiguous DXE heap.
  { "DXE Heap",      0x40440000ULL, 0x0C9C0000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN },

  // Linux-visible System RAM extents from the supplied RMX2151L1 /proc/iomem.
  // The malformed pasted 0x7DF00000..0x7DFFFFFF line is omitted pending correction.
  SYSTEM_RAM ("RAM 01", 0x4D100000ULL, 0x55FFFFFFULL),
  SYSTEM_RAM ("RAM 02", 0x56720000ULL, 0x7531FFFFULL),
  SYSTEM_RAM ("RAM 03", 0x75330000ULL, 0x75D2FFFFULL),
  SYSTEM_RAM ("RAM 04", 0x7E010000ULL, 0x7EE7FFFFULL),
  SYSTEM_RAM ("RAM 05", 0x7F400000ULL, 0x7F4BFFFFULL),
  SYSTEM_RAM ("RAM 06", 0x7F5C0000ULL, 0x7F5FFFFFULL),
  SYSTEM_RAM ("RAM 07", 0x80000000ULL, 0x8BFFFFFFULL),
  SYSTEM_RAM ("RAM 08", 0x8CD00000ULL, 0x8DFFFFFFULL),
  SYSTEM_RAM ("RAM 09", 0x8E900000ULL, 0x8EFFFFFFULL),
  SYSTEM_RAM ("RAM 10", 0x8F320000ULL, 0x9F5EFFFFULL),
  SYSTEM_RAM ("RAM 11", 0x9FF00000ULL, 0xBBFFFFFFULL),
  SYSTEM_RAM ("RAM 12", 0xBC400000ULL, 0xBCFFFFFFULL),
  SYSTEM_RAM ("RAM 13", 0xBD300000ULL, 0xBD513FFFULL),
  SYSTEM_RAM ("RAM 14", 0xC0000000ULL, 0xEFFFFFFFULL),
  // HakiOS BootShim copies an XNUP container here before entering the FD.
  // Keep the full 64 MiB out of DXE allocations and visible as reserved RAM.
  { "XNU Payload Reserved", 0xF0000000ULL, 0x04000000ULL, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN },
  SYSTEM_RAM ("RAM 15", 0xF4000000ULL, 0xFFCFFFFFULL),
  SYSTEM_RAM ("RAM 16", 0xFFE00000ULL, 0x13FFFEFFFULL),
  SYSTEM_RAM ("RAM 17", 0x140000000ULL, 0x23FFFDFFFULL),

  // Live-DT handoff regions; MMIO is kept distinct from ordinary RAM.
  MMIO_REGION ("GIC Distributor", 0x0C000000ULL, 0x00040000ULL),
  MMIO_REGION ("GIC Redistributor", 0x0C040000ULL, 0x00200000ULL),
  MMIO_REGION ("GIC Config", 0x0C53A650ULL, 0x00000050ULL),
  MMIO_REGION ("UART", 0x11002000ULL, 0x00001000ULL),
  MMIO_REGION ("UFS HCI", 0x11270000ULL, 0x00002300ULL),
  MMIO_REGION ("UFS PHY", 0x11FA0000ULL, 0x0000C000ULL),
  // Bootloader-owned display memory; SimpleFbDxe does not reinitialize DSI.
  { "Display Reserved", 0x75D30000ULL, 0x02CD0000ULL, NoHob, MMAP_IO, INITIALIZED, Conv, NS_DEVICE },
};

VOID
GetMemoryMap (
  OUT EFI_MEMORY_REGION_DESCRIPTOR **MemoryDescriptor,
  OUT UINT8                         *MemoryDescriptorCount
  )
{
  *MemoryDescriptor = mMemoryMap;
  *MemoryDescriptorCount = (UINT8)ARRAY_SIZE (mMemoryMap);
}
