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


### GCC Common Intermediate Language (CIL) Front End
The GCC front end for the Common Language Infrastructure (CLI) Common Intermediate Language (CIL)
was a project to take compiled CIL as input and generate machine-executable code as output.
After the front end was separated, the code was originally written against GCC 4.3 and then
later updated to GCC 4.5.

Unfortunately, GCC 4.3 has internal compiler error (ICE) issues (causing builds of GCC targeting
h8300-hitachi-coff to fail), and GCC 4.5 is _after_ “generic coff” support was dropped from GCC.
The patching done here attempts to get the Common Intermediate Language Front End (CIL FE)
working with that skipped version—GCC 4.4.

An extract of this CIL FE from the GCC repositories is at [BrickBot/gcc-cli-fe](https://github.com/BrickBot/gcc-cli-fe),
and the patches and files here are based on that code, and the branches in that respository
cover the following:

| Branch Name              | Description |
| ------------------------ | ----------- |
| master                   | The original, unedited branch with all files  |
| h8300-coff-base          | Branch of the last commit against GCC 4.3 before GCC was updated to 4.5 |
| h8300-coff               | Starting from branch `h8300-coff-base`, apply updates to make it buildable (but excluding libcilsupport) |
| h8300-coff-libcilsupport | Adds updates to libcilsupport from where branch `h8300-coff` left off |
| post-h8300-cli-fe-squash | Branch created for collapsing updates subsequent to the GCC 4.5 update into a single squash commit, from which patch files could then be generated |


Starting with the migration from GCC 4.3 to 4.5,
all subsequent CIL-specific commits have been captured as patch files.  This complete set of patches can
be found in the file `post-h8300-cli-fe-squash.patch`; however, to make it easier to work with the patches,
this single file was split into several separate files based on the impacted functionality:

| Individual Patch Files                              | Description |
| --------------------------------------------------- | ----------- |
| `post-h8300-cli-fe-squash-just-cil.patch`           | Changes to code under gcc/cil        |
| `post-h8300-cli-fe-squash-just-cil-parser.patch`    | Changes specific to gcc/cil/parser.c |
| `post-h8300-cli-fe-squash-just-gcc-common.patch`    | Changes to “common” files (e.g. Makefile, configure, gcc/common.opt, gcc/config/\*) |
| `post-h8300-cli-fe-squash-just-libcilsupport.patch` | Changes specific to libcilsupport    |


Additionally, there are several auto-generated \*.h files that unfortunately aren’t all
fully autogenerated under GCC 4.4.  In particular, the following files contained incomplete
definitions when autogenerated under GCC 4.4, so “static” copies were capture from a build of
the version based on GCC 4.5 and then referenced statically within the updates for GCC 4.4.

| Incomplete Dynamically-Generated File | `#include`’d by      |
| ------------------------------------- | -------------------- |
| `gt-cil-bindings.h`                   | `gcc/cil/bindings.c` |
| `gt-cil-stack.h`                      | `gcc/cil/stack.c`    |
| `gtype-cil.h`                         | `gcc/cil/cil1.c`     |



NewLib 1.19.0
-------------
While newlib-1.20.0 was technically the last version to support h8300-\*-coff,
version 1.20.0 introduces incompatibilities with version of libiberty used by the other tools supporting h8300-\*-coff
     (c.f. [gcc list](https://gcc-patches.gcc.gnu.narkive.com/zeSeZ9N8/newlib-vs-libiberty-mismatch-breaks-build-re-patch-export-psignal-on-all-platforms#post1)),
so newlib-1.19.0 is effectively the last version of NewLib to support h8300-\*-coff.

### Debian
While Debian did not provide a newlib-1.19.0 package, all
[patches from Debian’s packaging of newlib-1.18.0](https://sources.debian.org/src/newlib/1.18.0-6.2/debian/)
appear applicable to newlib-1.19.0 as well.

| Applied? | Patch File Name                | Notes |
| -------- | ------------------------------ | ----- |
| ✔       | 60_newlib-libgloss-eabi.patch  |       |
| ✔       | 61_newlib_arm_include.patch    |       |
| ✔       | 62_long_double_infinity.patch  |       |
| ✔       | 65_multiarch-headers.patch     |       |


GNU Debugger 6.8 (GDB)
----------------------
GDB 6.8 is able to be built together with the rest of the GNU toolchain as part of a combined build.

### Debian
The [patches listed below are from Debian’s packing of gdb-6.8](https://sources.debian.org/src/gdb/6.8-3/debian/patches/).
Patch order is based on the “series” file.

| Applied? | Patch File Name                       | Notes |
| -------- | ------------------------------------- | ----- |
| ✔       | 05_member-field-symtab.patch          | Patch for Debian bug #239535 |
|        | 10_thread-db-multiple-libraries.patch | Support loading two libthread_db DSOs |
| ❌       | ~~15_dwarf2-cfi-warning.patch~~       | Eliminate “noisy” CIE warning; not suitable for upstream |
| ❌       | ~~20_gdbinit-ownership.patch~~        | gdbinit; while concept seemed acceptable, implementation was not popular upstream |
|        | 25_gdb-pascal-support.patch           | Pascal support |
|        | 30_gdb-fortran-main.patch             | Set the main function in Fortran programs to "MAIN__" to help with recognizing as Fortran |
|        | 35_linux-clear-thread-list.patch      | Patch for Debian bug #303736, but the change might not be in the right place? |
|        | 40_man-page-args.patch                | Man page updates for arguments |
|        | 45_bfd-get-mtime-less.patch           | Updates for bfd mtime; applied upstream after GBD 6.8 |
|        | 50_gdb-rbreak-quoting.patch           | Quote symbol name before passing it to break_command; backported from trunk after GDB 6.8 |
|        | 55_ia64-array-bound.patch             | Correct array access |
|        | 60_cp-support-uninit.patch            | Initialize storage |
|        | 65_m68k-nat-build-fix.patch           | Fix typo |


GNU Debugger 7.12.1 (GDB)
-------------------------
The last version of GDB to technically support the h8300-hitachi-coff was 7.12.1.
While GDB 7.12.1 seems to indicate that h8300-\*-\*-coff targets are supported
(note lack of support for h8300-\*-\*-coff [covered by the "h8300-\*-\*-\*" case] in bfd/config.bfd in later versions),
in practice this is broken and will trigger various malloc errors on attempting to initiate a debug session.
GDB 6 has not exhibited this issue.

Additionally, GDB 7 is incompatible with the shared libraries that are part of the GNU toolchain and must be built separately,
but GDB 6 does maintain that compatibility and is able to built together with the rest of the GNU toolchain.

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
