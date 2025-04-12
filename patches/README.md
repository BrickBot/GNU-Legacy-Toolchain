Patches
=======
Known patches are listed below.  Those not marked with ~~strikethrough~~ have been applied to the code in this respository.
Unapplied patches might be specific to a particular distribution or duplicate patches from other sources.

Please note that these patches do _not_ reflect the only changes to the code.  Other changes were made as needed to
address various build issues and can be reviewed in the commit history.


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

| Applied? | Patch File Name                      | Notes |
| -------- | ------------------------------------ | ----- |
| ❌       | ~~000_print_debian_version~~         |       |
| ✔       | 002_gprof_profile_arcs               |       |
| ✔       | 003_gprof_see_also_monitor           |       |
| ✔       | 006_better_file_error                |       |
| ✔       | 012_check_ldrunpath_length           |       |
| ❌       | ~~121_i386_x86_64_biarch~~           |       |
| ❌       | ~~127_x86_64_i386_biarch~~           |       |
| ✔       | 128_ln_s_makefiles                   |       |
| ✔       | 129_gas_h8300                        |       |
| ✔       | 130_coff_arm                         |       |
| ✔       | 131_sprintf                          |       |
| ✔       | 132_texinfo_fixes                    |       |
| ❌       | ~~bts729274~~                        |       |

#### Patches from Debian release v-11 and forward
Patches maintained commencing with Debian release [2.16.1-11](https://sources.debian.org/src/binutils-h8300-hms/2.16.1-12/debian/patches/),
with patch descriptions [available here](https://udd.debian.org/patches.cgi?src=binutils-h8300-hms&version=2.16.1-12).

**TODO**:

| Applied? | Patch File Name                      | Notes |
| -------- | ------------------------------------ | ----- |
| ❌       | ~~000-print-debian-version.patch~~   | Was also included in v-10 |
| ❌       | ~~006-better-file-error.patch~~      | Was also included in v-10 |
| ❌       | ~~012-check-ldrunpath-length.patch~~ | Was also included in v-10 |
| ❌       | ~~128-ln-s-makefiles.patch~~         | Was also included in v-10 |
| ❌       | ~~129-gas-h8300.patch~~              | Was also included in v-10 |
| ❌       | ~~130-coff-arm.patch~~               | Was also included in v-10 |
| ❌       | ~~131-sprintf.patch~~                | Was also included in v-10 |
| ❌       | ~~132-texinfo-fixes.patch~~          | Was also included in v-10 |
|          | 133-configure-signatures.patch       | New subsequent to v-10 |
| ✔       | 134-texinfo-fixes.patch              | New subsequent to v-10 |
|          | 135-update-autotools.patch           | New subsequent to v-10 |
|          | 136-ld-libdir.patch                  | New subsequent to v-10 |
|          | 137-abort-on-invalid-header.patch    | New subsequent to v-10 |
|          | 138-ldflags.patch                    | New subsequent to v-10 |
|          | 139-spacepad.patch                   | New subsequent to v-10 |
| ❌       | ~~140-deterministic-output.patch~~   | New with v-13 |


GCC
----
### GNU GCC Post-Release Patches
Subsequent to the final release of GCC 3 ([v3.4.6 on March 6, 2006](https://gcc.gnu.org/develop.html)), four additional patches were pushed to the
[releases/gcc-3.4](https://github.com/gcc-mirror/gcc/commits/releases/gcc-3.4/) branch.  Patch files are available in both \*.diff and \*.patch forms.

| Applied? | Patch File Name | Notes |
| -------- | --------------- | ----- |
| ❌       | ~~m32r~~        | Three patches—all released later in 2006—pertain to m32r support but are superseded by more comprehensive patches from Debian (c.f. the Debian patch section). |
| ❌       | ~~bison~~       | The fourth patch—from 2009—provides a fix for compatibility with bison 2.4.1.  Superseded by the more recent pair of Bison patches from the Red Hat compatibility patches and Debian patches. |

### Modernization Compatibility Patches
* ✔ [Convert `struct ucontext` to `ucontext_t`](https://github.com/BrickBot/GNU-Legacy-Toolchain/issues/8)
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
* [Patch Ordering (spec file)](https://git.centos.org/rpms/compat-gcc-34/blob/c7/f/SPECS/compat-gcc-34.spec)
  + `update-siginfo.sh` script extracted from the spec file

| Applied? | Patch File Name                                    | Notes |
| -------- | -------------------------------------------------- | ----- |
| ❌       | 001-gcc34-multi32-hack.patch                       | Sparc optimization hack |
| ✔       | 002-gcc34-ice-hack.patch                           | GCC compiler exit codes |
| ✔       | 003-gcc34-ppc64-m32-m64-multilib-only.patch        | Config |
| ✔       | 004-gcc34-ia64-lib64.patch                         | Config |
| ✔       | 005-gcc34-java-nomulti.patch                       | Java  |
| ❌       | 006-gcc34-gnuc-rh-release.patch                    | Red Hat specific |
| ✔       | 007-gcc34-pr16104.patch                            | GCC   |
|          | 008-gcc34-var-tracking-fix.patch                   | Dwarf2 |
| ✔       | 009-gcc34-i386-movsi-insv.patch                    | Config |
| ❌       | 010-gcc34-pr18925.patch                            | C++ vars – creates undefined reference to “determine_visibility” |
| ✔       | 011-gcc34-pr14084.patch                            | GCC emit |
| ✔       | 012-gcc34-hashtab-recursion.patch                  | GCC vars |
| ✔       | 013-gcc34-java-jnilink.patch                       | Java  |
| ❌       | 014-gcc34-pr21955.patch                            | libstdc++-v3 – Missing files referenced in Makefile |
| ❌       | 015-gcc34-vsb-stack.patch                          | GCC – auto-patching failed |
| ✔       | 016-gcc34-pr18300.patch                            | Config |
| ✔       | 017-gcc34-rh156291.patch                           | GCC   |
| ✔       | 018-gcc34-weakref.patch                            | WeakRef |
|          | 019-gcc34-dwarf2-usefbreg.patch                    | Dwarf2 |
|          | 020-gcc34-dwarf2-prefer-1elt-vartracking.patch     | Dwarf2 |
|          | 021-gcc34-dwarf2-pr20268.patch                     | Dwarf2 |
|          | 022-gcc34-dwarf2-inline-details.patch              | Dwarf2 |
|          | 023-gcc34-dwarf2-frame_base.patch                  | Dwarf2 |
|          | 024-gcc34-dwarf2-i386-multreg1.patch               | Dwarf2 |
|          | 025-gcc34-dwarf2-i386-multreg2.patch               | Dwarf2 |
|          | 026-gcc34-rh176182.patch                           | Dwarf2 |
| ✔       | 027-gcc34-pr11953.patch                            | Config |
| ❌       | 028-gcc34-pr23591.patch                            | libstdc++-v3 – Unsupported on H8/300 |
|          | 029-gcc34-pr26208.patch                            | Unwind, impacting multiple: Config, Java, libstdc++-v3, GCC |
| ✔       | 030-gcc34-pr8788.patch                             | GCC   |
| ✔       | 031-gcc34-rh137200.patch                           | GCC   |
| ✔       | 032-gcc34-rh172117.patch                           | GCC   |
|          | 033-gcc34-rh172876.patch                           | Dwarf2 |
| ✔       | 034-gcc34-rh178062.patch                           | GCC vars |
| ✔       | 035-gcc34-pr21412.patch                            | Config |
| ❌       | 036-gcc34-sw2438.patch                             | GCC vars – auto-patching failed |
|          | 037-gcc34-pr26208-workaround.patch                 | Unwind |
| ✔       | 038-gcc34-libgcc_eh-hidden.patch                   | libgcc |
|          | 039-gcc34-frame-base-loclist.patch                 | Dwarf2 + Unwind |
| ✔       | 040-gcc34-CVE-2006-3619.patch                      | Java  |
|          | 041-gcc34-dwarf2-inline-details-fix.patch          | Dwarf2 |
| ✔       | 042-gcc34-CXXABI131.patch                          | libstdc++-v3 |
| ✔       | 043-gcc34-rh205919.patch                           | GCC   |
| ✔       | 044-gcc34-rh207277.patch                           | GCC   |
| ❌       | 045-gcc34-var-tracking-coalesce.patch              | GCC vars – a file to patch (var-tracking.c) does not exist |
| ✔       | 046-gcc34-java-zoneinfo.patch                      | Java  |
| ✔       | 047-gcc34-libgcc-additions.patch                   | libgcc |
| ✔       | 048-gcc34-pr24975.patch                            | libstdc++-v3 |
| ✔       | 049-gcc34-rh233941.patch                           | GCC/g77 |
| ✔       | 050-gcc34-rh234515.patch                           | libstdc++-v3 |
| ✔       | 051-gcc34-rh235008.patch                           | GCC   |
| ✔       | 052-gcc34-rh235255.patch                           | GCC   |
| ✔       | 053-gcc34-rh242685.patch                           | libstdc++-v3 |
| ❌       | 100-gcc34-ldbl-hack.patch                          | libstdc++-v3 |
| ✔       | 101-gcc34-makeinfo.patch                           | Config (for Makeinfo 4.10+) |
| ✔       | 102a-gcc34-bison4.patch                            | Fixes for Bison breaking changes |
| ✔       | 102b-gcc34-bison.patch                             | Fixes for Bison breaking changes |
| ✔       | 103-gcc34-pr56258.patch                            | Address Makeinfo v5+ documentation errors |
|          | dummylib.sh                                        |       |
| ✔       | update-siginfo.sh                                  | `struct siginfo` to `siginfo_t` |

### Debian
Debian provides a [gcc-h8300-hms](https://packages.debian.org/source/stable/gcc-h8300-hms) package
based on gcc 3.4.6 that bundles several patches.
From the `patches/series` file in that archive (c.f. [release versions here](https://sources.debian.org/src/gcc-h8300-hms/3.4.6/debian/patches/series/)),
available Debian patches include those lised below.  Use [this link](https://sources.debian.org/src/gcc-h8300-hms/stable/debian/patches/) to be redirected to the latest collection of stable patches.
Generally, unapplied patches are more Debian specific.
#### Patches through Debian release dfsg2-4.2
Numerous patches were maintained up through Debian release [dfsg2-4.2](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-4.2/debian/patches/),
with patch descriptions [available here](https://udd.debian.org/patches.cgi?src=gcc-h8300-hms&version=1%3A3.4.6%2Bdfsg2-4.2).

| Applied? | Patch File Name                      | Notes |
| -------- | ------------------------------------ | ----- |
| ❌       | ~~000_no_libstdc++-v3~~              |       |
| ✔       | 001_h8300_64bit                      |       |
| ✔       | gcc-textdomain.dpatch                |       |
| ❌       | ~~libstdc++-pic.dpatch~~             |       |
| ✔       | libstdc++-doclink.dpatch             |       |
| ✔       | libstdc++-doxygen-syntax.dpatch      |       |
| ✔       | amd64-specs.dpatch                   |       |
| ✔       | gccbug.dpatch                        |       |
| ✔       | gccbug-posix.dpatch                  |       |
| ✔       | gcc-textdomain.dpatch                |       |
| ✔       | hppa-toplevel.dpatch                 |       |
| ✔       | m68k-update.dpatch                   |       |
| ✔       | arm-bigendian.dpatch                 |       |
| ✔       | cpu-default-i486.dpatch              |       |
| ✔       | deb-protoize.dpatch                  |       |
| ✔       | hurd-changes.dpatch                  |       |
| ✔       | m32r-gotoff.dpatch                   |       |
| ✔       | m32r-stack.dpatch                    |       |
| ✔       | m32r-fixes.dpatch                    |       |
| ✔       | m32r-limits.dpatch                   |       |
| ✔       | m32r-pie.dpatch                      |       |
| ✔       | m32r-remove-addsi3.dpatch            |       |
| ✔       | m32r-auto.dpatch                     |       |
| ✔       | alpha-ieee.dpatch                    |       |
| ✔       | libstdc++-mips-atomic.dpatch         |       |
| ✔       | bin_false_fixup                      |       |
| ❌       | ~~configure-update~~                 |       |
| ❌       | ~~debian-changes-1:3.4.6+dfsg-1~~    |       |
| ✔       | fix_ftbs.diff                        |       |

#### Patches from Debian release dfsg2-5 and forward
Commencing with Debian release [dfsg2-5](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-5/debian/patches/), the number of patches was reduced, while a few new ones were added.
Patch descriptions for these are [available here](https://udd.debian.org/patches.cgi?src=gcc-h8300-hms&version=1%3A3.4.6%2Bdfsg2-5).

**TODO**:

| Applied? | Patch File Name                                   | Notes |
| -------- | ------------------------------------------------- | ----- |
| ❌       | ~~000_no_libstdc%2B%2B-v3~~                       | Was also included in dfsg2-4.2 |
| ❌       | ~~001_h8300_64bit~~                               | Was also included in dfsg2-4.2 |
| ❌       | ~~gcc-textdomain.dpatch~~                         | Was also included in dfsg2-4.2 |
| ❌       | ~~bin_false_fixup~~                               | Was also included in dfsg2-4.2 |
| ❌       | ~~debian-changes-1%3A3.4.6%2Bdfsg-1~~             | Was also included in dfsg2-4.2 |
| ❌       | ~~fix_ftbs.diff~~                                 | Was also included in dfsg2-4.2 |
| ❌       | ~~150-configure-signatures.patch~~                              | New subsequent to dfsg2-4.2 |
| ✔       | 0030-Add-now-mandatory-parameter-to-AM_GNU_GETTEXT.patch        | New subsequent to dfsg2-4.2 |
|          | 0032-Fix-format-not-a-string-literal.patch                      | New subsequent to dfsg2-4.2 |
|          | 0033-Remove-AC_CONFIG_AUX_DIR-in-libiberty-configure-temp.patch | New subsequent to dfsg2-4.2 |
| ✔       | 0034-Fix-c-parse.y-build-error.patch                            | New subsequent to dfsg2-4.2 |

### GNU Pascal Compiler (GPC)
The GNU Pascal Compiler (GPC) [patch](https://github.com/hebisch/gpc/blob/master/p/diffs) most applicable to this particular GCC version were also applied.
The other patches for more general GCC modernization (such as for multiarch and ucontext, as noted in the [GPC ReadMe](https://github.com/hebisch/gpc)) are covered by other patches and updates.

| Applied? | Patch File Name | Notes |
| -------- | --------------- | ----- |
| ✔       | gcc-3.4.4.diff  |       |


GNU Debugger (GDB)
------------------
The last version of GDB to support the h8300-hitachi-coff was 7.12.1.
### Debian
Debian never released a corresponding package for GDB 7.12.1 (c.f. [list](https://sources.debian.org/src/gdb/)),
but it did maintain a package for [version 7.12](https://sources.debian.org/src/gdb/7.12-6/) which included a number of patches
(c.f. [patch descriptions](https://sources.debian.org/patches/gdb/7.12-6/) and [patch ordering (series file)](https://sources.debian.org/src/gdb/7.12-6/debian/patches/series/)).

| Applied? | Patch File Name                                                     | Notes |
| -------- | ------------------------------------------------------------------- | ----- |
| ✔       | gdb-fortran-main.patch                                              |       |
| ✔       | solve_PATH_MAX_issue.patch                                          |       |
| ✔       | gdb-6.5-bz185337-resolve-tls-without-debuginfo-v2.patch             |       |
| ✔       | python-config.patch                                                 |       |
| ✔       | gdb-glibc-vdso-workaround.patch                                     |       |
| ❌       | load-versioned-libcc1.patch                                         |       |
|          |                                                                     |       |
|          | **Hurd Support – Upstream Backports**                               |       |
| ✔       | 0001-Hurd-Adjust-to-Per-inferior-Inferior-qualified-threa.patch     |       |
| ❌       | ~~0003-Hurd-C-Explicitly-cast-void.patch~~                          | Commented out in series file |
| ✔       | 0004-Hurd-C-Avoid-GNU-C-nested-functions.patch                      |       |
| ✔       | 0005-Hurd-C-Avoid-const-char-to-char-casts.patch                    |       |
| ✔       | 0006-Hurd-C-kern_return_t-vs.-error_t.patch                         |       |
| ✔       | 0007-Hurd-C-Mach-Hurd-headers-and-MIG-stubs-are-not-yet-f.patch     |       |
| ✔       | 0008-Hurd-In-the-CLI-use-parse_thread_id-instead-of-globa.patch     |       |
| ✔       | 0021-Avoid-PATH_MAX-usage.patch                                     |       |
| ✔       | 0022-Hurd-Adjust-to-changes-to-push-pruning-old-threads-d.patch     |       |
| ✔       | explicit-cast-void.patch                                            |       |
| ✔       | hurd-backport-for-7.12-branch.patch                                 |       |
|          |                                                                     |       |
|          | **From gdb-7.12-branch**                                            |       |
| ❌       | 0001-Document-the-GDB-7.12-release-in-gdb-ChangeLog.patch           | Patch already applied in 7.12.1 |
| ❌       | 0002-Bump-GDB-version-number-to-7.12.0.DATE-git.patch               | Patch already applied in 7.12.1 |
| ❌       | 0008-AArch64-Track-FP-registers-in-prologue-analyzer.patch          | Patch already applied in 7.12.1 |
| ❌       | 0011-Include-strings.h-where-available.patch                        | Patch already applied in 7.12.1 |
| ❌       | 0022-GDBserver-Fix-conversion-warning.patch                         | Patch already applied in 7.12.1 |
| ❌       | 0023-PR-gdb-20653-small-cleanup-in-string_to_explicit_loc.patch     | Patch already applied in 7.12.1 |
| ❌       | 0025-Fix-gdb-C-compilation-on-Solaris-PR-build-20712.patch          | Patch already applied in 7.12.1 |
| ❌       | 0026-Added-forgotten-gdb-ChangeLog-entry.patch                      | Patch already applied in 7.12.1 |
| ❌       | 0044-sim-mips-fix-builds-for-r3900-cpus-due-to-missing-ch.patch     | Patch already applied in 7.12.1 |
| ❌       | 0045-sim-mips-fix-dv-tx3904cpu-build-error.patch                    | Patch already applied in 7.12.1 |
| ❌       | 0047-sim-mips-add-PR-info-to-ChangeLog.patch                        | Patch already applied in 7.12.1 |
| ❌       | 0075-Create-tdep-rl78_psw_type-lazily.patch                         | Patch already applied in 7.12.1 |
| ❌       | 0076-Create-tdep-rx_psw_type-and-tdep-rx_fpsw_type-lazily.patch     | Patch already applied in 7.12.1 |
| ❌       | 0080-Remove-assert-on-exec_bfd-in-cris_delayed_get_disass.patch     | Patch already applied in 7.12.1 |
| ❌       | 0089-Fix-longjmp-across-readline-w-enable-sjlj-exceptions.patch     | Patch already applied in 7.12.1 |
| ❌       | 0090-gdb-Fix-C-and-C-03-builds.patch                                | Patch already applied in 7.12.1 |
| ❌       | 0115-Fix-some-error-handling-bugs-in-python-frame-filters.patch     | Patch already applied in 7.12.1 |
