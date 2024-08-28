H8/300 COFF (HMS) Toolchain
===========================
A cross-toolchain targeting Hitachi H8/300 processors with the COFF (or, colloquially, HMS) format, including binutils, gcc, gpc, gdb, and newlib.


Repository Creation Notes
-------------------------

### Sources Versions Used
Last known versions to include support for h8300-*-coff
* BinUtils 2.16.1
  + Note lack of support for h8300-*-coff in gas/configure.tgt in later versions
* GCC 3.4.6
  + Last supported version is actually the GCC 4.4 series – note lack of support for h8300-*-coff (covered by the "h8300-*-*" case) in libgcc/config.host in later versions
  + So why GCC 3.4.6?
    - H8/300 was supported for the duration of the full GCC version series, with 3.4.6 [closing the release series](https://gcc.gnu.org/gcc-3.4/changes.html)
    - Source code has been suppored by Debian package maintainers with maintance patches (gcc-h8300 package)
    - GPC integration is less robust in GCC 4
    - Created smaller binaries compared to the same builds created using GCC 4
      * Especially important on memory-constrained devices such as the LEGO MindStorms RCX
  + Resources
    - [Installation configuration documentation](https://web.archive.org/web/20041013092023/https://gcc.gnu.org/install/configure.html)
    - [Manual](https://gcc.gnu.org/onlinedocs/gcc-3.4.6/gcc/)
* GPC – note the included README files
* GDB 7.12.1 – note lack of support for h8300-*-*-coff (covered by the "h8300-*-*-*" case) in bfd/config.bfd in later versions
* NewLib 1.19.0
  + Version 1.20.0 introduces incompatibilities with the binutil's version of libiberty (c.f. [gcc list](https://gcc-patches.gcc.gnu.narkive.com/zeSeZ9N8/newlib-vs-libiberty-mismatch-breaks-build-re-patch-export-psignal-on-all-platforms#post1))
  + Versions 2.0 and later fail to build if targeting h8300-*-coff



### Combined Folder Composition
Several folders are duplicated across the various project comprising the toolchain.
Unfortunately, given the differing ages of the various releases,
it does not seem that they can be readily combined into a single build.

Taking libiberty from binutils and the combining the gcc and binutils builds does seem to work,
which at least eliminates the complications that would otherwise arise from having to build
gcc and binutils separately.

| Folder    | gcc | binutils | newlib | gdb |
| --------- | --- | -------- | ------ | --- |
| bfd       |     |  ×       |        |  ×  |
| cpu       |     |  ×       |        |  ×  |
| etc       |     |  ×       |  ×     |  ×  |
| include   |  ×  |  ×       |        |  ×  |
| libiberty |  ×  |  ×       |        |  ×  |
| opcodes   |     |  ×       |        |  ×  |
| texinfo   |     |  ×       |  ×     |  ×  |
| zlib      |  ×  |          |        |  ×  |




The following were soft-linked in to create the combined source folder `src-combined`,
which is then used as the source folder for builds:

Folders from GCC
* boehm-gc
* config
* fastjar
* gcc
* include
* intl
* libf2c
* libffi
* libjava
* libobjc
* libstdc++-v3
* zlib

Folders from BinUtils
* bfd
* binutils
* cgen
* cpu
* etc
* gas
* gprof
* ld
* libiberty
* opcodes
* texinfo

Folders from GPC
* p -> gcc/p

Files from GCC
* config.guess
* config.if
* config.rpath
* config.sub
* config-ml.in
* configure
* configure.in
* install-sh
* libtool.m4
* ltcf-c.sh
* ltcf-cxx.sh
* ltcf-gcj.sh
* ltconfig
* ltmain.sh
* Makefile.def
* Makefile.in
* Makefile.tpl
* missing
* mkinstalldirs
* move-if-change
* symlink-tree
* ylwrap


The following were not able to be linked in as part of teh combined sources folder
without also introducing build issues (these can build separately, though):

Folders from GDB
* gdb
* libdecnumber
* readline
* sim

Folders from NewLib
* libgloss
* newlib



### Patches
The following patches were applied.

#### Binutils
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


#### GCC
Debian provides a [gcc-h8300-hms](https://packages.debian.org/source/stable/gcc-h8300-hms) package
based on gcc 3.4.6 that includes several patch updates in the debian.tar.xz archive file available on that page.
From the `patches/series` file in that archive (c.f. [release versions here](https://sources.debian.org/src/gcc-h8300-hms/3.4.6/debian/patches/series/)),
available Debian patches include those lised below.
Patches not marked with strikethrough have been applied here (unapplied patches are more Debian specific).
* 000_no_libstdc++-v3
* 001_h8300_64bit
* gcc-textdomain.dpatch
* libstdc++-pic.dpatch
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

The GNU Pascal Compiler (GPC) patches most applicable to this particular GCC version were also applied.
* gcc-3.4.4.diff
