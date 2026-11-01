##
# SALAA / Realme 7 RMX2151 / MediaTek MT6785.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
import logging
import os

from edk2toolext.environment import shell_environment
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.invocables.edk2_pr_eval import PrEvalSettingsManager
from edk2toolext.invocables.edk2_setup import RequiredSubmodule, SetupSettingsManager
from edk2toolext.invocables.edk2_update import UpdateSettingsManager
from edk2toolext.invocables.edk2_parse import ParseSettingsManager


class CommonPlatform:
    PackagesSupported = ("SalaaPkg",)
    ArchSupported = ("AARCH64",)
    TargetsSupported = ("DEBUG", "RELEASE")
    Scopes = ("salaa", "gcc_aarch64_linux", "edk2-build")
    WorkspaceRoot = os.path.dirname(
        os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    )
    PackagesPath = (
        "Platforms/Realme",
        "Common/Mu",
        "Common/Mu_OEM_Sample",
        "Mu_Basecore",
        "Silicon/MediaTek",
        "Silicon/Silicium",
    )


class SettingsManager(
    UpdateSettingsManager, SetupSettingsManager, PrEvalSettingsManager, ParseSettingsManager
):
    def GetPackagesSupported(self):
        return CommonPlatform.PackagesSupported

    def GetArchitecturesSupported(self):
        return CommonPlatform.ArchSupported

    def GetTargetsSupported(self):
        return CommonPlatform.TargetsSupported

    def GetRequiredSubmodules(self):
        return [
            RequiredSubmodule("Binaries", True),
            RequiredSubmodule("Common/Mu", True),
            RequiredSubmodule("Common/Mu_OEM_Sample", True),
            RequiredSubmodule("Mu_Basecore", True),
            RequiredSubmodule("Silicium-ACPI", True),
            RequiredSubmodule(
                "Silicon/Silicium/OpensslPkg/Library/OpensslLib/openssl", True
            ),
        ]

    def SetArchitectures(self, requested):
        unsupported = set(requested) - set(self.GetArchitecturesSupported())
        if unsupported:
            raise ValueError("Unsupported architecture(s): " + ", ".join(sorted(unsupported)))
        self.ActualArchitectures = requested

    def GetWorkspaceRoot(self):
        return CommonPlatform.WorkspaceRoot

    def GetActiveScopes(self):
        return CommonPlatform.Scopes

    def GetPlatformDscAndConfig(self):
        # Use workspace-relative paths here.  Mu's OverrideValidation plugin
        # resolves DSC library INF paths through Edk2Path; it does not resolve
        # this package alias consistently when the package is overlayed by CI.
        return ("Platforms/Realme/SalaaPkg/SalaaPkg.dsc", {})

    def GetName(self):
        return "Salaa"

    def GetPackagesPath(self):
        return CommonPlatform.PackagesPath


class PlatformBuilder(UefiBuilder, BuildSettingsManager):
    def __init__(self):
        UefiBuilder.__init__(self)

    def GetWorkspaceRoot(self):
        return CommonPlatform.WorkspaceRoot

    def GetPackagesPath(self):
        result = [shell_environment.GetBuildVars().GetValue("FEATURE_CONFIG_PATH", "")]
        return result + list(CommonPlatform.PackagesPath)

    def GetActiveScopes(self):
        return CommonPlatform.Scopes

    def GetName(self):
        return "SalaaPkg"

    def SetPlatformEnv(self):
        self.env.SetValue("PRODUCT_NAME", "SALAA", "Platform")
        self.env.SetValue(
            "ACTIVE_PLATFORM", "Platforms/Realme/SalaaPkg/SalaaPkg.dsc", "Platform"
        )
        self.env.SetValue("TARGET_ARCH", "AARCH64", "Platform")
        self.env.SetValue("TOOL_CHAIN_TAG", "CLANGPDB", "Mu-Silicium")
        self.env.SetValue("EMPTY_DRIVE", "FALSE", "Platform")
        self.env.SetValue("RUN_TESTS", "FALSE", "Platform")
        self.env.SetValue("SHUTDOWN_AFTER_RUN", "FALSE", "Platform")
        self.env.SetValue("BLD_*_BUILDID_STRING", "Unknown", "Platform")
        self.env.SetValue("BUILDREPORTING", "TRUE", "Platform")
        self.env.SetValue(
            "BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Platform"
        )
        self.env.SetValue("BLD_*_MEMORY_PROTECTION", "TRUE", "Platform")
        self.env.SetValue("BLD_*_SHIP_MODE", "FALSE", "Platform")
        for name in ("ENABLE_SECUREBOOT", "FD_BASE", "FD_SIZE", "FD_BLOCKS"):
            self.env.SetValue(f"BLD_*_{name}", self.env.GetValue(name), "Mu-Silicium")
        return 0

    def PlatformPreBuild(self):
        return 0

    def PlatformPostBuild(self):
        return 0

    def FlashRomImage(self):
        return 0


if __name__ == "__main__":
    import argparse
    import sys
    from edk2toolext.invocables.edk2_platform_build import Edk2PlatformBuild
    from edk2toolext.invocables.edk2_setup import Edk2PlatformSetup
    from edk2toolext.invocables.edk2_update import Edk2Update

    script_path = os.path.relpath(__file__)
    parser = argparse.ArgumentParser(add_help=False)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--update", "--UPDATE", action="store_true")
    group.add_argument("--setup", "--SETUP", action="store_true")
    args, remaining = parser.parse_known_args()
    sys.argv = ["stuart", "-c", script_path] + remaining

    if args.setup:
        Edk2PlatformSetup().Invoke()
    elif args.update:
        Edk2Update().Invoke()
    else:
        Edk2PlatformBuild().Invoke()
