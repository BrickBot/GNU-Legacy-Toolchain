Patches
=======
Known patches are listed below.  Those not marked with ~~strikethrough~~ have been applied to the code in this respository.
Unapplied patches might be specific to a particular distribution or duplicate patches from other sources.


Binutils
--------
### Debian
Debian provides a [binutils-h8300-hms](https://packages.debian.org/source/stable/binutils-h8300-hms) package
based on binutils 2.16.1 that bundles several patches.
From the `patches/series` file in that archive (c.f. [release versions here](https://sources.debian.org/src/binutils-h8300-hms/2.16.1/debian/patches/series/)),
available Debian patches include those lised below.
Use [this link](https://sources.debian.org/src/binutils-h8300-hms/stable/debian/patches/) to be redirected to the latest collection of stable patches.
Generally, unapplied patches are more Debian specific.
#### Patches through Debian release v-10
Patches maintained up through Debian release [2.16.1-10](https://sources.debian.org/src/binutils-h8300-hms/2.16.1-10/debian/patches/),
with patch descriptions [available here](https://udd.debian.org/patches.cgi?src=binutils-h8300-hms&version=2.16.1-10).
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
#### Patches from Debian release v-11 and forward
Patches maintained commencing with Debian release [2.16.1-11](https://sources.debian.org/src/binutils-h8300-hms/2.16.1-12/debian/patches/),
with patch descriptions [available here](https://udd.debian.org/patches.cgi?src=binutils-h8300-hms&version=2.16.1-12).

**TODO**:

| Patch File Name | Notes |
| --------------- | ----- |
| ~~000-print-debian-version.patch~~   | Was also included in v-10 |
| ~~006-better-file-error.patch~~      | Was also included in v-10 |
| ~~012-check-ldrunpath-length.patch~~ | Was also included in v-10 |
| ~~128-ln-s-makefiles.patch~~         | Was also included in v-10 |
| ~~129-gas-h8300.patch~~              | Was also included in v-10 |
| ~~130-coff-arm.patch~~               | Was also included in v-10 |
| ~~131-sprintf.patch~~                | Was also included in v-10 |
| ~~132-texinfo-fixes.patch~~          | Was also included in v-10 |
| 133-configure-signatures.patch       | New subsequent to v-10 |
| 134-texinfo-fixes.patch              | New subsequent to v-10 |
| 135-update-autotools.patch           | New subsequent to v-10 |
| 136-ld-libdir.patch                  | New subsequent to v-10 |
| 137-abort-on-invalid-header.patch    | New subsequent to v-10 |
| 138-ldflags.patch                    | New subsequent to v-10 |
| 139-spacepad.patch                   | New subsequent to v-10 |


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
based on gcc 3.4.6 that bundles several patches.
From the `patches/series` file in that archive (c.f. [release versions here](https://sources.debian.org/src/gcc-h8300-hms/3.4.6/debian/patches/series/)),
available Debian patches include those lised below.  Use [this link](https://sources.debian.org/src/gcc-h8300-hms/stable/debian/patches/) to be redirected to the latest collection of stable patches.
Generally, unapplied patches are more Debian specific.
#### Patches through Debian release dfsg2-4.2
Numerous patches were maintained up through Debian release [dfsg2-4.2](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-4.2/debian/patches/),
with patch descriptions [available here](https://udd.debian.org/patches.cgi?src=gcc-h8300-hms&version=1%3A3.4.6%2Bdfsg2-4.2).
* ~~000_no_libstdc++-v3~~
* 001_h8300_64bit
* gcc-textdomain.dpatch
* ~~libstdc++-pic.dpatch~~
* libstdc++-doclink.dpatch
* libstdc++-doxygen-syntax.dpatch
* amd64-specs.dpatch
* gccbug.dpatch
* gccbug-posix.dpatch
* gcc-textdomain.dpatch
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
#### Patches from Debian release dfsg2-5 and forward
Commencing with Debian release [dfsg2-5](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-5/debian/patches/), the number of patches was reduced, while a few new ones were added.
Patch descriptions for these are [available here](https://udd.debian.org/patches.cgi?src=gcc-h8300-hms&version=1%3A3.4.6%2Bdfsg2-5).

**TODO**:

| Patch File Name | Notes |
| --------------- | ----- |
| ~~000_no_libstdc%2B%2B-v3~~                       | Was also included in dfsg2-4.2 |
| ~~001_h8300_64bit~~                               | Was also included in dfsg2-4.2 |
| ~~gcc-textdomain.dpatch~~                         | Was also included in dfsg2-4.2 |
| ~~bin_false_fixup~~                               | Was also included in dfsg2-4.2 |
| ~~debian-changes-1%3A3.4.6%2Bdfsg-1~~             | Was also included in dfsg2-4.2 |
| ~~fix_ftbs.diff~~                                 | Was also included in dfsg2-4.2 |
| 150-configure-signatures.patch                                  | New subsequent to dfsg2-4.2 |
| 0030-Add-now-mandatory-parameter-to-AM_GNU_GETTEXT.patch        | New subsequent to dfsg2-4.2 |
| 0032-Fix-format-not-a-string-literal.patch                      | New subsequent to dfsg2-4.2 |
| 0033-Remove-AC_CONFIG_AUX_DIR-in-libiberty-configure-temp.patch | New subsequent to dfsg2-4.2 |
| 0034-Fix-c-parse.y-build-error.patch                            | New subsequent to dfsg2-4.2 |

### GNU Pascal Compiler (GPC)
The GNU Pascal Compiler (GPC) patches most applicable to this particular GCC version were also applied.
* gcc-3.4.4.diff
