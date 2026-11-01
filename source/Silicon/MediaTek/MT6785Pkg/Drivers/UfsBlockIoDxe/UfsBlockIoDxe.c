/** @file
  MT6785 UFS Block I/O DXE stub — SALAA / RMX2151.

  This driver registers an EFI_BLOCK_IO_PROTOCOL instance backed by the
  MediaTek MT6785 UFS host controller at 0x11270000 and the UFS M-PHY at
  0x11fa0000 (confirmed from the live device tree "ufshci@11270000" and
  "ufs_mphy@11fa0000" nodes).

  STATUS: This is a FUNCTIONAL STUB.  It publishes the Block I/O protocol
  with the correct geometry derived from device-tree evidence so that UEFI
  infrastructure (Shell, BDS, HakiOS XnuLoader) can enumerate Block I/O
  handles and locate the boot partition table.  The underlying UFS host +
  PHY initialisation sequence (clock enabling, UICC link startup, UFSHCIv2
  task-management ring, power mode negotiation, LUN access) is NOT
  implemented; all read/write requests return EFI_NO_MEDIA until a full
  MT6785 UFS host driver is added.

  Why the stub is useful:
   - Provides the correct EFI handle/protocol for BDS device-order lists.
   - Lets the EDK II Shell enumerate drives and exercise the Media.LastBlock
     field without attempting actual disk I/O.
   - Proves the protocol path in CI; a full driver replaces only the
     ReadBlocks/WriteBlocks implementations.

  UFS geometry (from device-tree and standard UFS spec defaults):
   - LUN count: 8 (UFS 2.1, all LUNs active by default)
   - Block size: 4096 bytes (UFS logical block size)
   - Capacity: unknown until LUN INQUIRY/READ CAPACITY succeeds; stub
     reports 0 blocks (Media.LastBlock = 0, Media.MediaPresent = FALSE).

  References:
   - "ufshci@11270000" in stock-salaa-1.dts (reg 0x11270000, size 0x2300)
   - "ufs_mphy@11fa0000" in stock-salaa-1.dts (reg 0x11fa0000, size 0xc000)
   - JEDEC UFS 2.1 spec (JESD220C)
   - Linux kernel: drivers/ufs/host/ufs-mediatek.c
   - Linux kernel: drivers/phy/mediatek/phy-mtk-ufs.c

  Copyright (c) 2024, SALAA UEFI Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/BlockIo.h>

/* UFS HCI and PHY base addresses (DT-confirmed) */
#define MT6785_UFS_HCI_BASE   0x11270000ULL
#define MT6785_UFS_PHY_BASE   0x11fa0000ULL

/* Number of UFS LUNs exposed (UFS 2.1 default) */
#define MT6785_UFS_LUN_COUNT  8U

/* Standard UFS logical block size */
#define MT6785_UFS_BLOCK_SIZE 4096U

/* Revision string */
#define MT6785_UFS_REVISION   L"MT6785 UFS Stub v0.1"

typedef struct {
  UINTN                    Signature;
  EFI_BLOCK_IO_PROTOCOL    BlockIo;
  EFI_BLOCK_IO_MEDIA       Media;
  UINTN                    Lun;
} SALAA_UFS_DEV;

#define SALAA_UFS_SIGNATURE  SIGNATURE_32 ('S','U','F','S')
#define SALAA_UFS_FROM_BLKIO(a) \
  CR (a, SALAA_UFS_DEV, BlockIo, SALAA_UFS_SIGNATURE)

/* ------------------------------------------------------------------ */
/*  Block I/O protocol stubs                                           */
/* ------------------------------------------------------------------ */

STATIC
EFI_STATUS
EFIAPI
UfsReset (
  IN EFI_BLOCK_IO_PROTOCOL   *This,
  IN BOOLEAN                  ExtendedVerification
  )
{
  /* PHY/link init is not implemented; return success to avoid BDS stall. */
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
UfsReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  )
{
  /*
   * Full UFS host driver not implemented.  Report EFI_NO_MEDIA so that
   * callers can distinguish "stub present, hardware not init'd" from a
   * protocol-absent condition.
   */
  return EFI_NO_MEDIA;
}

STATIC
EFI_STATUS
EFIAPI
UfsWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                  MediaId,
  IN EFI_LBA                 Lba,
  IN UINTN                   BufferSize,
  IN VOID                   *Buffer
  )
{
  return EFI_WRITE_PROTECTED;
}

STATIC
EFI_STATUS
EFIAPI
UfsFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

/* ------------------------------------------------------------------ */
/*  Driver entry point                                                 */
/* ------------------------------------------------------------------ */

EFI_STATUS
EFIAPI
UfsBlockIoDxeInitialise (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS     Status;
  SALAA_UFS_DEV *Dev;
  UINTN          Lun;
  EFI_HANDLE     Handle;

  DEBUG ((DEBUG_INFO,
    "MT6785 UFS stub: HCI=0x%08llX PHY=0x%08llX LUNs=%u\n",
    (UINT64)MT6785_UFS_HCI_BASE,
    (UINT64)MT6785_UFS_PHY_BASE,
    MT6785_UFS_LUN_COUNT));

  for (Lun = 0; Lun < MT6785_UFS_LUN_COUNT; Lun++) {
    Dev = AllocateZeroPool (sizeof (SALAA_UFS_DEV));
    if (Dev == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Dev->Signature = SALAA_UFS_SIGNATURE;
    Dev->Lun       = Lun;

    /* Media descriptor — no media until host driver initialises the link */
    Dev->Media.MediaId          = (UINT32)Lun;
    Dev->Media.RemovableMedia   = FALSE;
    Dev->Media.MediaPresent     = FALSE;
    Dev->Media.LogicalPartition = FALSE;
    Dev->Media.ReadOnly         = FALSE;
    Dev->Media.WriteCaching     = FALSE;
    Dev->Media.BlockSize        = MT6785_UFS_BLOCK_SIZE;
    Dev->Media.IoAlign          = 4;
    Dev->Media.LastBlock        = 0;

    /* Block I/O protocol */
    Dev->BlockIo.Revision    = EFI_BLOCK_IO_PROTOCOL_REVISION3;
    Dev->BlockIo.Media       = &Dev->Media;
    Dev->BlockIo.Reset       = UfsReset;
    Dev->BlockIo.ReadBlocks  = UfsReadBlocks;
    Dev->BlockIo.WriteBlocks = UfsWriteBlocks;
    Dev->BlockIo.FlushBlocks = UfsFlushBlocks;

    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gEfiBlockIoProtocolGuid, &Dev->BlockIo,
                    NULL);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "MT6785 UFS stub: LUN%u install failed: %r\n",
        Lun, Status));
      FreePool (Dev);
      /* Continue for remaining LUNs */
    } else {
      DEBUG ((DEBUG_INFO, "MT6785 UFS stub: LUN%u handle installed.\n", Lun));
    }
  }

  return EFI_SUCCESS;
}
