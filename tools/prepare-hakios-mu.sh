#!/usr/bin/env bash
# Assemble the exact HakiOS UEFI patch stack plus the SALAA platform overlay.
# SPDX-License-Identifier: MIT
set -euo pipefail

PORT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HAKIOS_ROOT="${1:?usage: prepare-hakios-mu.sh HAKIOS_UEFI_CHECKOUT MU_CHECKOUT}"
MU_ROOT="${2:?usage: prepare-hakios-mu.sh HAKIOS_UEFI_CHECKOUT MU_CHECKOUT}"
source "$PORT_ROOT/integrations/hakios/lock.env"

actual_hakios="$(git -C "$HAKIOS_ROOT" rev-parse HEAD)"
actual_mu="$(git -C "$MU_ROOT" rev-parse HEAD)"
[[ "$actual_hakios" == "$HAKIOS_UEFI_COMMIT" ]] || {
  echo "HakiOS UEFI checkout is $actual_hakios; expected $HAKIOS_UEFI_COMMIT" >&2
  exit 1
}
[[ "$actual_mu" == "$MU_COMMIT" ]] || {
  echo "Mu-Silicium checkout is $actual_mu; expected $MU_COMMIT" >&2
  exit 1
}

git -C "$MU_ROOT" apply --check --whitespace=nowarn "$HAKIOS_ROOT/hakios.patch"
git -C "$MU_ROOT" apply --whitespace=nowarn "$HAKIOS_ROOT/hakios.patch"
cp -a "$HAKIOS_ROOT/files/." "$MU_ROOT/"
python3 "$HAKIOS_ROOT/branding/banner.py" \
  "$MU_ROOT/Silicon/Silicium/SiliciumPkg/Include/Resources"
cp -a "$PORT_ROOT/source/." "$MU_ROOT/"
install -D "$PORT_ROOT/Resources/Configs/Salaa.toml" \
  "$MU_ROOT/Resources/Configs/Salaa.toml"

for file in \
  BootShim/BootShim.S \
  Silicon/Silicium/SiliciumPkg/Library/BootManagerLib/XnuLoader.c \
  Silicon/Silicium/SiliciumPkg/Library/BootManagerLib/XnuLoader.h \
  Platforms/Realme/SalaaPkg/SalaaPkg.dsc \
  Platforms/Realme/SalaaPkg/SalaaPkg.fdf \
  Platforms/Realme/SalaaPkg/Library/MemoryMapLib/MemoryMapLib.c; do
  test -f "$MU_ROOT/$file" || { echo "Missing integration input: $file" >&2; exit 1; }
done

echo "Prepared SALAA HakiOS UEFI tree"
echo "HakiOS UEFI: $actual_hakios"
echo "Mu-Silicium: $actual_mu"
