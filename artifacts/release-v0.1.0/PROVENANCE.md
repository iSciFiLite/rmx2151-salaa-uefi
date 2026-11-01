# SALAA UEFI v0.1.0 release artifacts

These files are preserved from the published [`v0.1.0` release archive](https://github.com/iSciFiLite/rmx2151-salaa-uefi/releases/download/v0.1.0/salaaPkg-v0.1.0.zip). They are the prior local EDK II build outputs, renamed to distinguish that release build from future GitHub Actions builds.

- Release archive: `salaaPkg-v0.1.0.zip`
- Release archive size: 522,198 bytes
- Release archive SHA-256: `bfb8dd33d8082a2a3d81bce43e3e6c8e82b4fa900208566a4268e34f0d93dba1`
- Release tag target: `5627806bf65b89d1c23ba191f5499708fda6cdb2`
- Historical UEFI image: `SALAA_EFI.release-v0.1.0.fd`
- Image size: 4,194,304 bytes (4 MiB)
- Image SHA-256: `5e6ab9e0e9376b634e5393b39b9889c671e746a49f3f582c90b858b75bf1ace7`
- Recorded output directory: `Build/SALAA/DEBUG_GCC/FV/`
- Historical toolchain/build label: EDK II, `DEBUG_GCC`

Other generated outputs from the same archive are retained here with the same release suffix: SEC `.efi` files, the SEC `.dll`, and linker map. The original archive remains attached to the GitHub Release unchanged.

The release records a successful EDK II build in the original Ubuntu workspace. The preserved image has not been validated on an RMX2151 or shown to provide GOP, PSCI, UFS Block I/O, or a safe HakiOS memory handoff. It is not interchangeable with a future Actions-built image, which will use a separate filename and provenance record.
