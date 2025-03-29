Patches
=======
The patches described below have been applied to the code in this respository.


Binutils
--------
Debian provides a [binutils-h8300-hms](https://packages.debian.org/source/stable/binutils-h8300-hms) package
based on binutils 2.16.1 that includes several patch updates in the debian.tar.xz archive file available on that page.
From the `patches/series` file in that archive (c.f. [release versions here](https://sources.debian.org/src/binutils-h8300-hms/2.16.1/debian/patches/series/)),
available Debian patches include those lised below.
Patches not marked with strikethrough have been applied here (unapplied patches are more Debian specific).
* ~~000_print_debian_version~~
* 002_gprof_profile_arcs
* 003_gprof_see_also_monitor
* 006_better_file_error
* 012_check_ldrunpath_length
* ~~121_i386_x86_64_biarch~~
* ~~127_x86_64_i386_biarch~~
* 128_ln_s_makefiles
* 129_gas_h8300
* 130_coff_arm
* 131_sprintf
* 132_texinfo_fixes
* ~~bts729274~~


GCC
----
### GNU GCC Post-Release Patches
Subsequent to the final release of GCC 3 ([v3.4.6 on March 6, 2006](https://gcc.gnu.org/develop.html)), four additional patches were pushed to the
[releases/gcc-3.4](https://github.com/gcc-mirror/gcc/commits/releases/gcc-3.4/) branch.  Patch files are available in both \*.diff and \*.patch forms.
* ~~m32r~~: Three patches—all released later in 2006—pertain to m32r support but are superseded by more comprehensive patches from Debian (c.f. the Debian patch section).
* ~~bison~~: The fourth patch—from 2009—provides a fix for compatibility with bison 2.4.1.  Superseded by the more recent pair of Bison patches from the Red Hat compatibility patches.

### Modernization Compatibility Patches
* [Convert `struct ucontext` to `ucontext_t`](https://github.com/BrickBot/GNU-Legacy-Toolchain/issues/8)
  + `struct ucontext` has been replaced by `ucontext_t` in more recent releases.
  + GCC 3.4.6 needs to be updated accordingly in order to build for impacted targeted systems (e.g. not needed for targeting h8300-hitachi-coff but is needed for targeting x86-64).
  + Patches
    - [More comprehensive patch](https://patchwork.ozlabs.org/project/gcc/patch/alpine.DEB.2.20.1706271356170.15648@digraph.polyomino.org.uk/#1713139)
      * [Later patch discussion](https://gcc.gnu.org/pipermail/gcc-patches/2025-February/676093.html)
    - [Basic patch for x86-64 PCs](https://github.com/hebisch/gpc/blob/master/p/diffs/ucontext.diff)
      * c.f. [GPC ReadMe](https://github.com/hebisch/gpc)


### Red Hat
Red Hat provided [backwards compatibility patches for GCC 3.4](https://access.redhat.com/solutions/19458).
* [Patch Sources](https://git.centos.org/rpms/compat-gcc-34/blob/c7/f/SOURCES)
    (c.f. [GitHub clone, which includes an additional Bison patch](https://github.com/weevington/gcc-3.4.6-41-compat))
* [Patch Ordering](https://git.centos.org/rpms/compat-gcc-34/blob/c7/f/SPECS/compat-gcc-34.spec)

### Debian
Debian provides a [gcc-h8300-hms](https://packages.debian.org/source/stable/gcc-h8300-hms) package
based on gcc 3.4.6 that includes several patch updates in the debian.tar.xz archive file available on that page.
From the `patches/series` file in that archive (c.f. [release versions here](https://sources.debian.org/src/gcc-h8300-hms/3.4.6/debian/patches/series/)),
available Debian patches include those lised below.
Patches not marked with strikethrough have been applied here (unapplied patches are more Debian specific).
* ~~000_no_libstdc++-v3~~
* 001_h8300_64bit
* gcc-textdomain.dpatch
* ~~libstdc++-pic.dpatch~~
* libstdc++-doclink.dpatch
* libstdc++-doxygen-syntax.dpatch
* amd64-specs.dpatch
* gccbug.dpatch
* gccbug-posix.dpatch
* hppa-toplevel.dpatch
* m68k-update.dpatch
* arm-bigendian.dpatch
* cpu-default-i486.dpatch
* deb-protoize.dpatch
* hurd-changes.dpatch
* m32r-gotoff.dpatch
* m32r-stack.dpatch
* m32r-fixes.dpatch
* m32r-limits.dpatch
* m32r-pie.dpatch
* m32r-remove-addsi3.dpatch
* m32r-auto.dpatch
* alpha-ieee.dpatch
* libstdc++-mips-atomic.dpatch
* bin_false_fixup
* ~~configure-update~~
* ~~debian-changes-1:3.4.6+dfsg-1~~
* fix_ftbs.diff

### GNU Pascal Compiler (GPC)
The GNU Pascal Compiler (GPC) patches most applicable to this particular GCC version were also applied.
* gcc-3.4.4.diff
