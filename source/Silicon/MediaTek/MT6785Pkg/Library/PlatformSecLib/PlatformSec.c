/**
  SALAA early platform hook.
  No undocumented MT6785 register writes are performed here. Keep the hook
  inert until the corresponding clock, watchdog, and security handoff are
  measured on the target board.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>
#include <Library/PlatformSecLib.h>

VOID
PlatformInitialize (
  VOID
  )
{
  // SEC entry is intentionally read-only with respect to SoC control registers.
}
