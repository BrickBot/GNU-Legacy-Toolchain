GNU Legacy Toolchain
====================
A legacy GNU toolchain that includes binutils, gcc, gpc, gdb, and newlib.

Multiple use cases exist for continuing to maintain a legacy toolchain:
* **H8/300 Processor Targets**
* **GNU Pascal Compiler (GPC)**
* **Fortran77 / G77 Compiler**

While primarily intended for building a cross-toolchain targeting Hitachi/Renesas H8/300 processors with the COFF (or, colloquially, HMS) format, this should work for other targets as well.

While this H8/300 target has sometimes been referred to as h8300-hitachi-hms, it is [more properly](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-4.2/debian/rules/#L30) identified as h8300-hitachi-coff.


Repository Creation Notes
-------------------------

### Sources Versions Used
The primary selection criteria was the last known versions to include support for h8300-\*-coff,
but this also overlapped well with support for GPC and g77.
* **BinUtils 2.16.1**
  + Note lack of support for h8300-\*-coff in gas/configure.tgt in later versions
* **GCC 3.4.6**  (c.f. [full GCC release timeline](https://gcc.gnu.org/develop.html))
  + Last supported version is actually the GCC 4.4 series – note lack of support for h8300-\*-coff (covered by the "h8300-\*-\*" case) in libgcc/config.host in later versions
  + So why GCC 3.4.6?
    - H8/300 was supported for the duration of the full GCC version series, with 3.4.6 [closing the release series](https://gcc.gnu.org/gcc-3.4/changes.html)
    - GPC integration is less robust in GCC 4
    - Fortran77/g77 support was not included in later GCC versions, and some older code is not well suited to being built by newer Fortran compilers such as `gfortran` ([ref 1](https://github.com/weevington/gcc-3.4.6-41-compat), [ref 2](https://forums.linuxmint.com/viewtopic.php?t=261066)).
    - Created smaller binaries compared to the same builds created using GCC 4
      * Especially important on memory-constrained devices such as the LEGO MindStorms RCX
    - Multiple ongoing patch sources were provided by Linux distributions such as RedHad and Debian
  + Resources
    - [Build and Installation Configuration Documentation](https://web.archive.org/web/20041013092023/https://gcc.gnu.org/install/configure.html)
    - [Manual](https://gcc.gnu.org/onlinedocs/gcc-3.4.6/gcc/)
      * [Manual subsets and/or other formats](https://gcc.gnu.org/onlinedocs/) (scroll down for the GCC 3.4.6 manuals section)
* **GPC 2.1-20070904** – note the included README files
  + [The GNU Pascal Manual](https://www.gnu-pascal.de/gpc/)
  + [GPC website](https://www.gnu-pascal.de/gpc/h-index.html)
    - [Compilation and installation guide](https://www.gnu-pascal.de/gpc/Compiling-GPC.html#Compiling-GPC):  Note the section covering `pascal.install` at the end
    - [Cross-compilation guide](https://www.gnu-pascal.de/gpc/Cross_002dCompilers.html#Cross_002dCompilers)
  + Source files from [hebisch/gpc](https://github.com/hebisch/gpc)
  + [Mailing list](https://www.gnu.de/mailman3/hyperkitty/list/gpc@gnu.de/latest) (no longer seems to be active)
    - [Subscribe/Unsubscribe page](https://www.gnu.de/mailman3/postorius/lists/gpc.gnu.de/)
* **GDB 7.12.1** – note lack of support for h8300-\*-\*-coff (covered by the "h8300-\*-\*-\*" case) in bfd/config.bfd in later versions
* **NewLib 1.19.0**
  + Version 1.20.0 introduces incompatibilities with the binutil’s version of libiberty (c.f. [gcc list](https://gcc-patches.gcc.gnu.narkive.com/zeSeZ9N8/newlib-vs-libiberty-mismatch-breaks-build-re-patch-export-psignal-on-all-platforms#post1))
  + Versions 2.0 and later fail to build if targeting h8300-\*-coff



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
