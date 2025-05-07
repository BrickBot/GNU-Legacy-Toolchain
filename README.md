GNU Legacy Toolchain
====================
A legacy GNU toolchain that includes binutils, gcc, gpc, gdb, and newlib.

Multiple use cases exist for continuing to maintain a legacy toolchain:
* **COFF targets, such as for the H8/300 Processor**
* **GNU Pascal Compiler (GPC)**
* **Fortran77 / G77 Compiler**

While primarily intended for building a cross-toolchain targeting Hitachi/Renesas H8/300 processors with the COFF (or, colloquially, HMS) format, this should work for other targets as well.

While this H8/300 target has sometimes been referred to as h8300-hitachi-hms, it is more properly identified as
h8300-hitachi-coff ([ref 1](https://tracker.debian.org/pkg/gcc-h8300-hms), [ref 2](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-4.2/debian/rules/#L30)).


Repository Creation Notes
-------------------------

### Sources Versions Used
The primary selection criteria was the last known versions to include support for h8300-\*-coff,
but this also overlapped well with support for GPC and g77.
* **[BinUtils](https://www.gnu.org/software/binutils/) 2.16.1**  (released 2005-06-12 [c.f. ChangeLog files for bfd, gas, and ld])
  + Note lack of support for h8300-\*-coff in gas/configure.tgt in later versions
* **GCC 3.4.6**  (released 2006-03-06 [c.f. [full GCC release timeline](https://gcc.gnu.org/develop.html)])
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
* **[GCC CIL Front End](https://gcc.gnu.org/projects/cli.html) 4.3.0-2007-12-13 (final commit [2011-06-20](https://gcc.gnu.org/git/?p=gcc.git;a=shortlog;h=refs/vendors/st/heads/cli-fe))**
  + Work was done on separate branches
  + A fork was made for the front end
    - Initial split was done based on GCC 4.3.0 but then updated to something post GCC 4.4 (h8300-\*-coff) no longer supported
    - Started from the last commit based on GCC 4.3.0 and then worked forward
* **GCC 4.4.7**  (released 2012-03-13 [c.f. [full GCC release timeline](https://gcc.gnu.org/develop.html)])
  + Last version to support h8300-\*-coff (must use the `--enable-obsolete` flag when configuring)
  + Support for “Generic COFF” in general was [dropped following the GCC 4.4 release series](https://gcc.gnu.org/gcc-4.4/changes.html)
* **GPC 2.1-20070904** – note the included README files
  + [The GNU Pascal Manual](https://www.gnu-pascal.de/gpc/)
  + [GPC website](https://www.gnu-pascal.de/gpc/h-index.html)
    - [Compilation and installation guide](https://www.gnu-pascal.de/gpc/Compiling-GPC.html#Compiling-GPC):  Note the section covering `pascal.install` at the end
    - [Cross-compilation guide](https://www.gnu-pascal.de/gpc/Cross_002dCompilers.html#Cross_002dCompilers)
  + Source files from [hebisch/gpc](https://github.com/hebisch/gpc)
  + [Mailing list](https://www.gnu.de/mailman3/hyperkitty/list/gpc@gnu.de/latest) (no longer seems to be active)
    - [Subscribe/Unsubscribe page](https://www.gnu.de/mailman3/postorius/lists/gpc.gnu.de/)
* **[NewLib](https://sourceware.org/newlib/) 1.19.0** (released 2010-12-16 [c.f. Downloads > Snapshots > http link within web frames])
  + Version 1.20.0 introduces incompatibilities with the binutil’s version of libiberty (c.f. [gcc list](https://gcc-patches.gcc.gnu.narkive.com/zeSeZ9N8/newlib-vs-libiberty-mismatch-breaks-build-re-patch-export-psignal-on-all-platforms#post1))
  + Versions 2.0 and later fail to build if targeting h8300-\*-coff
* **[GDB](https://sourceware.org/gdb/) 7.12.1** ([released 2017-01-21](https://sourceware.org/gdb/schedule/))– note lack of support for h8300-\*-\*-coff (covered by the "h8300-\*-\*-\*" case) in bfd/config.bfd in later versions


### Updates and Modifications
A more in-depth description is available in the [patches folder](patches/).


### Combined Folder Composition
Several folders are duplicated across the various project comprising the toolchain.
Unfortunately, given the differing ages of the various releases,
it does not seem that they can be readily combined into a single build.

Taking libiberty from binutils and the combining the gcc and binutils builds does seem to work,
which at least eliminates the complications that would otherwise arise from having to build
gcc and binutils separately.

| Folder       | gcc 3.4.6 | gcc-cil-fe 4.3.0 | gcc 4.4.7 | binutils | newlib | gdb |
| ------------ | --------- | ---------------- | --------- | -------- | ------ | --- |
| bfd          |           |                  |           |  ×       |        |  ×  |
| cpu          |           |                  |           |  ×       |        |  ×  |
| etc          |           |                  |           |  ×       |  ×     |  ×  |
| include      |  ×        |  ×               |  ×        |  ×       |        |  ×  |
| intl         |  ×        |  ×               |  ×        |  ×       |        |  ×  |
| libdecnumber |           |  ×               |  ×        |          |        |  ×  |
| libiberty    |  ×        |  ×               |  ×        |  ×       |        |  ×  |
| opcodes      |           |                  |           |  ×       |        |  ×  |
| texinfo      |           |                  |           |  ×       |  ×     |  ×  |
| zlib         |  ×        |  ×               |  ×        |          |        |  ×  |



The following folders were soft-linked in to create the corresponding directory
under the combined source folder (`src-combined`),
which is then used as the source folder for builds:

| Folder       | Source     | GCC3-Based Version | GCC4.4-Based Version |
| ------------ | ---------- | ------------------ | -------------------- |
| bfd          | binutils   | 2.16.1             | 2.16.1               |
| binutils     | binutils   | 2.16.1             | 2.16.1               |
| boehm-gc     | gcc        | 3.4.6              | 4.4.7                |
| cgen         | binutils   | 2.16.1             | 2.16.1               |
| config       | gcc        | 3.4.6              | 4.4.7                |
| cpu          | binutils   | 2.16.1             | 2.16.1               |
| etc          | binutils   | 2.16.1             | 2.16.1               |
| fastjar      | gcc        | 3.4.6              | 4.4.7                |
| gas          | binutils   | 2.16.1             | 2.16.1               |
| gcc          | gcc        | 3.4.6              | 4.4.7                |
| gcc/p        | GPC        | 2.1-20070904       | 2.1-20070904         |
| gprof        | binutils   | 2.16.1             | 2.16.1               |
| include      | GCC (libs) | 4.4.7              | 4.4.7                |
| intl         | GCC (libs) | 4.4.7              | 4.4.7                |
| ld           | binutils   | 2.16.1             | 2.16.1               |
| libf2c       | gcc        | 3.4.6              | 4.4.7                |
| libffi       | gcc        | 3.4.6              | 4.4.7                |
| libgloss     | newlib     | 1.19.0             | 1.19.0               |
| libiberty    | GCC (libs) | 4.4.7              | 4.4.7                |
| libjava      | gcc        | 3.4.6              | 4.4.7                |
| libobjc      | gcc        | 3.4.6              | 4.4.7                |
| libstdc++-v3 | gcc        | 3.4.6              | 4.4.7                |
| newlib       | newlib     | 1.19.0             | 1.19.0               |
| opcodes      | binutils   | 2.16.1             | 2.16.1               |
| texinfo      | binutils   | 2.16.1             | 2.16.1               |
| zlib         | GCC (libs) | 4.4.7              | 4.4.7                |

The soft-linked files under that same folder are all from GCC:
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


The following were not able to be linked in as part of the combined sources folder
without also introducing build issues (these can build separately, though):

Folders from GDB
* gdb
* libdecnumber
* readline
* sim
