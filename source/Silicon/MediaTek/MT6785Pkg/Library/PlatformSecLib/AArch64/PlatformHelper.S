//
// MT6785 SALAA SEC assembly initialization hook.
// SPDX-License-Identifier: BSD-2-Clause-Patent
//
#include <AsmMacroLib.h>

ASM_FUNC (PlatformAssemblyInitialize)
  // No MT6785-specific assembly initialization is required here.
  // Preserve the entry state; SEC continues in the current exception level.
  ret
