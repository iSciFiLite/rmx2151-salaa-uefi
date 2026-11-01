/** @file
  MT6785 (Helio G90T) UART0 SerialPortLib — MMIO implementation.

  SALAA / RMX2151 console UART is the SoC UART0 at physical address
  0x11002000 (DT node "serial@11002000", compatible "mediatek,mt6577-uart").
  It is an 8250-compatible UART with MediaTek-specific high-speed extensions.

  The LK bootloader leaves UART0 configured at 115200-8N1 with the 26 MHz
  reference clock (UART_CLK = 26 000 000 Hz).  This driver re-initialises
  the UART to the same parameters so that it works whether LK left it open
  or not.

  Register layout (each index is word-wide; byte-offset = index * 4):
    0x00  RBR/THR   Rx/Tx FIFO
    0x01  IER       Interrupt enable
    0x02  IIR/FCR   FIFO control / interrupt ID
    0x03  LCR       Line control (bit7 = DLAB)
    0x04  MCR       Modem control
    0x05  LSR       Line status   (bit5 = THRE = Tx FIFO empty)
    0x06  MSR       Modem status
    0x07  SCR       Scratch
    --- accessible only when LCR bit7 (DLAB) = 1 ---
    0x00  DLL       Divisor latch LSB
    0x01  DLM       Divisor latch MSB
    --- MTK extended regs (always accessible via high-speed mode) ---
    0x09  HIGHS     High-speed mode enable   (0x3 = mode 3)
    0x0A  SAMPLE_CNT  Sample count
    0x0B  SAMPLE_PT   Sample point

  For 115200 baud from 26 MHz using MTK high-speed mode 3:
    DIVISOR = ceil(26000000 / (256 * 115200)) = 1  → DLL=1, DLM=0
    SAMPLE_CNT = (26000000 / (115200 * 1)) - 1 = 225
    SAMPLE_PT  = SAMPLE_CNT / 2 - 1 = 111

  Copyright (c) 2024, SALAA UEFI Contributors. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <Library/SerialPortLib.h>
#include <Library/IoLib.h>

/*
 * UART0 physical base.  This is a fixed PCDs-free constant; the
 * address is set by the SoC and confirmed by the live device tree.
 */
#define SALAA_UART0_BASE   0x11002000ULL

/* Register word-stride: MT6785 UART registers are at offset = index * 4 */
#define UREG(index)        ((UINTN)(SALAA_UART0_BASE) + ((index) * 4U))

/* Standard 8250 register indices */
#define UART_RBR    0U   /* Rx Buffer (DLAB=0)        */
#define UART_THR    0U   /* Tx Holding (DLAB=0)       */
#define UART_DLL    0U   /* Divisor LSB (DLAB=1)      */
#define UART_IER    1U   /* Interrupt Enable (DLAB=0) */
#define UART_DLM    1U   /* Divisor MSB (DLAB=1)      */
#define UART_IIR    2U   /* Interrupt ID (read)       */
#define UART_FCR    2U   /* FIFO Control (write)      */
#define UART_LCR    3U   /* Line Control              */
#define UART_MCR    4U   /* Modem Control             */
#define UART_LSR    5U   /* Line Status               */

/* MTK extended register indices (byte-offset = index * 4) */
#define MTK_UART_HIGHS       0x09U  /* High-speed mode           */
#define MTK_UART_SAMPLE_CNT  0x0AU  /* Sample count register     */
#define MTK_UART_SAMPLE_PT   0x0BU  /* Sample point register     */

/* LSR bits */
#define LSR_DR    BIT0   /* Data ready                */
#define LSR_THRE  BIT5   /* Transmitter holding empty */

/* LCR values */
#define LCR_8N1   0x03U  /* 8-bit, no parity, 1 stop  */
#define LCR_DLAB  BIT7   /* Divisor latch access      */

/* FCR: enable FIFO, clear Rx+Tx */
#define FCR_ENABLE (BIT0 | BIT1 | BIT2)

/*
 * Baud-rate parameters for 115200 from 26 MHz, MTK high-speed mode 3.
 * HIGHS = 3 selects the fractional-divider path.
 * DIVISOR = ceil(F_ref / (256 * baud)) = 1
 * SAMPLE_CNT = (F_ref / (baud * DIVISOR)) - 1 = 225
 * SAMPLE_PT  = SAMPLE_CNT / 2 - 1 = 111
 */
#define BAUD_HIGHS       3U
#define BAUD_DLL         1U
#define BAUD_DLM         0U
#define BAUD_SAMPLE_CNT  225U
#define BAUD_SAMPLE_PT   111U

STATIC BOOLEAN mInitialised = FALSE;

/**
  Initialise the MT6785 UART0 at 115200-8N1.

  @retval RETURN_SUCCESS  Always succeeds (MMIO write-only path).
**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  if (mInitialised) {
    return RETURN_SUCCESS;
  }

  /* 1. Disable interrupts */
  MmioWrite32 (UREG (UART_IER), 0);

  /* 2. Enable MTK high-speed mode before touching divisors */
  MmioWrite32 (UREG (MTK_UART_HIGHS), BAUD_HIGHS);

  /* 3. Set baud rate: open divisor latch */
  MmioWrite32 (UREG (UART_LCR), LCR_DLAB);
  MmioWrite32 (UREG (UART_DLL), BAUD_DLL);
  MmioWrite32 (UREG (UART_DLM), BAUD_DLM);
  /* Close divisor latch, set 8N1 */
  MmioWrite32 (UREG (UART_LCR), LCR_8N1);

  /* 4. Set high-speed sample count and sample point */
  MmioWrite32 (UREG (MTK_UART_SAMPLE_CNT), BAUD_SAMPLE_CNT);
  MmioWrite32 (UREG (MTK_UART_SAMPLE_PT),  BAUD_SAMPLE_PT);

  /* 5. Enable and reset FIFOs */
  MmioWrite32 (UREG (UART_FCR), FCR_ENABLE);

  mInitialised = TRUE;
  return RETURN_SUCCESS;
}

/**
  Write bytes to the UART.  Blocks until each byte is accepted by the
  transmitter holding register (LSR.THRE = 1).

  @param[in] Buffer    Source data.
  @param[in] NumberOfBytes  Bytes to transmit.

  @return  NumberOfBytes (always writes all bytes or spins forever).
**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8  *Buffer,
  IN UINTN   NumberOfBytes
  )
{
  UINTN Index;

  if (Buffer == NULL || NumberOfBytes == 0) {
    return 0;
  }

  for (Index = 0; Index < NumberOfBytes; Index++) {
    /* Spin until transmit holding register is empty */
    while ((MmioRead32 (UREG (UART_LSR)) & LSR_THRE) == 0) {
      /* busy-wait */
    }
    MmioWrite32 (UREG (UART_THR), (UINT32)Buffer[Index]);
  }

  return NumberOfBytes;
}

/**
  Read bytes from the UART receive FIFO (non-blocking).

  @param[out] Buffer        Destination buffer.
  @param[in]  NumberOfBytes Maximum bytes to read.

  @return  Actual bytes placed in Buffer (may be 0).
**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8  *Buffer,
  IN  UINTN   NumberOfBytes
  )
{
  UINTN Count = 0;

  if (Buffer == NULL) {
    return 0;
  }

  while (Count < NumberOfBytes) {
    if ((MmioRead32 (UREG (UART_LSR)) & LSR_DR) == 0) {
      break;
    }
    Buffer[Count++] = (UINT8)(MmioRead32 (UREG (UART_RBR)) & 0xFFU);
  }

  return Count;
}

/**
  Poll the UART for a received byte without consuming it.

  @retval TRUE   At least one byte is waiting.
  @retval FALSE  Receive FIFO is empty.
**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return (MmioRead32 (UREG (UART_LSR)) & LSR_DR) != 0;
}

/**
  Set new attributes on the serial device.

  @param[in] Control  Control bits to set.

  @retval RETURN_SUCCESS      The new attributes were set.
  @retval RETURN_UNSUPPORTED  Operation not supported.
**/
RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32  Control
  )
{
  return RETURN_UNSUPPORTED;
}

/**
  Retrieve the status of the control bits on a serial device.

  @param[out] Control  A pointer to return the current control signals.

  @retval RETURN_SUCCESS      The control bits were read.
  @retval RETURN_UNSUPPORTED  Operation not supported.
**/
RETURN_STATUS
EFIAPI
SerialPortGetControl (
  OUT UINT32  *Control
  )
{
  return RETURN_UNSUPPORTED;
}

/**
  Sets the baud rate, receive FIFO depth, transmit/receive time out, parity,
  data bits, and stop bits on a serial device.

  @param[in,out] BaudRate           The requested baud rate.
  @param[in,out] ReceiveFifoDepth   The requested depth of the FIFO on receive side.
  @param[in,out] Timeout            The requested time out for a single character in microseconds.
  @param[in,out] Parity             The type of parity to use.
  @param[in,out] DataBits           The number of data bits to use.
  @param[in,out] StopBits           The number of stop bits to use.

  @retval RETURN_SUCCESS            The new attributes were set.
  @retval RETURN_UNSUPPORTED        Operation not supported.
**/
RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64              *BaudRate,
  IN OUT UINT32              *ReceiveFifoDepth,
  IN OUT UINT32              *Timeout,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  )
{
  if (ReceiveFifoDepth != NULL) { *ReceiveFifoDepth = 32; }
  if (Parity           != NULL) { *Parity           = NoParity; }
  if (DataBits         != NULL) { *DataBits          = 8; }
  if (StopBits         != NULL) { *StopBits          = OneStopBit; }
  if (BaudRate         != NULL) { *BaudRate          = 115200; }
  return RETURN_SUCCESS;
}
