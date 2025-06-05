GNU Legacy Toolchain
====================
A legacy GNU toolchain that includes binutils, gcc, gpc, gdb, and newlib.

Multiple use cases exist for continuing to maintain a legacy toolchain:
* **COFF targets, such as for the H8/300 Processor**
* **GNU Pascal Compiler (GPC)**
* **Fortran77 / G77 Compiler**

While primary goal of this respository is to build a cross-toolchain targeting Hitachi/Renesas H8/300 processors with the COFF format, this should work for other targets as well.
Though this H8/300 target has colloquially been referred to as h8300-hitachi-hms, it is more properly identified as
h8300-hitachi-coff ([ref 1](https://tracker.debian.org/pkg/gcc-h8300-hms), [ref 2](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-4.2/debian/rules/#L30)).


Repository Creation Notes
-------------------------

### Sources Versions Used
The primary selection criteria was the last known versions to include support for h8300-\*-coff,
but this also overlapped well with support for GPC and Fortran77/g77.

| Project  | Version  | Release Date | High-Level Notes |
| -------- | -------- | ------------ | ---------------- |
| [BinUtils](https://gnu.org/software/binutils/) | 2.16.1   | [2005-06-12](https://sourceware.org/pub/binutils/releases/) ¹ | Note lack of support for h8300-\*-coff in gas/configure.tgt in later versions |
| [GCC](https://gnu.org/software/gcc/)      | 3.4.6    | [2006-03-06](https://gcc.gnu.org/develop.html) ² | Last _full_ version series to support h8300-\*-coff |
| [GCC](https://gnu.org/software/gcc/)      | 4.4.7    | [2012-03-13](https://gcc.gnu.org/develop.html) ² | Support for “Generic COFF” in general was [dropped following the GCC 4.4 release series](https://gcc.gnu.org/gcc-4.4/changes.html) |
| [GCC CIL Front End](https://gcc.gnu.org/projects/cli.html) | 4.3.0-2007-12-13 | [2011-06-20](https://gcc.gnu.org/git/?p=gcc.git;a=shortlog;h=refs/vendors/st/heads/cli-fe) (final commit) | Earlier coding was against GCC 4.3 and later updated to GCC 4.5 |
| [GPC](https://www.gnu-pascal.de/gpc/h-index.html)      | 2.1-20070904 | 2007-09-04 | See the included README files for further details |
| [NewLib](https://sourceware.org/newlib/)  | 1.19.0 ⁴ | [2010-12-16](https://sourceware.org/newlib/) ⁵ | Versions 2.0 and later fail to build if targeting h8300-\*-coff |
| [GDB](https://sourceware.org/gdb/)      | 6.8      | [released 2008-02-29](https://sourceware.org/gdb/schedule/) | While GDB 7.12.1 seems to indicate that h8300-\*-\*-coff targets are supported (note lack of support for h8300-\*-\*-coff [covered by the "h8300-\*-\*-\*" case] in bfd/config.bfd in later versions), in practice this is broken and will trigger various malloc errors on attempting to initiate a debug session. |

Additional Notes:
 ¹ BinUtils: For the BinUtils release date, see also the ChangeLog files for bfd, gas, and ld.
 ² GCC: Link includes the full GCC release timeline.
 ³ GCC 4.4.7: As the last GCC release to support h8300-\*-coff (c.f. also the lack of support for h8300-\*-coff [covered by the "h8300-\*-\*" case] in libgcc/config.host in later versions), the flag `--enable-obsolete` must be used when configuring.
 ⁴ NewLib: Version 1.20.0 introduces incompatibilities with libiberty 
     (c.f. [gcc list](https://gcc-patches.gcc.gnu.narkive.com/zeSeZ9N8/newlib-vs-libiberty-mismatch-breaks-build-re-patch-export-psignal-on-all-platforms#post1)).
 ⁵ NewLib:  Release history link opens in a subframe; at the site,
     navigate to [Download](https://sourceware.org/newlib/download.html) > [Snapshots](https://sourceware.org/ftp/newlib/index.html) to view the release timeline history subframe.

* **[GCC CIL Front End](https://gcc.gnu.org/projects/cli.html) 4.3.0-2007-12-13 (final commit [2011-06-20](https://gcc.gnu.org/git/?p=gcc.git;a=shortlog;h=refs/vendors/st/heads/cli-fe))**
  + Work was done on separate branches
  + A fork was made for the front end
    - Initial split was done based on GCC 4.3.0 but then updated to something post GCC 4.4 (h8300-\*-coff) no longer supported
    - Started from the last commit based on GCC 4.3.0 and then worked forward
* **GPC 2.1-20070904** – note the included README files
  + [The GNU Pascal Manual](https://www.gnu-pascal.de/gpc/)
  + [GPC website](https://www.gnu-pascal.de/gpc/h-index.html)
    - [Compilation and installation guide](https://www.gnu-pascal.de/gpc/Compiling-GPC.html#Compiling-GPC):  Note the section covering `pascal.install` at the end
    - [Cross-compilation guide](https://www.gnu-pascal.de/gpc/Cross_002dCompilers.html#Cross_002dCompilers)
  + Source files from [hebisch/gpc](https://github.com/hebisch/gpc)
  + [Mailing list](https://www.gnu.de/mailman3/hyperkitty/list/gpc@gnu.de/latest) (no longer seems to be active)
    - [Subscribe/Unsubscribe page](https://www.gnu.de/mailman3/postorius/lists/gpc.gnu.de/)


#### GCC 3.4.6 versus GCC 4.4.7
Why GCC 3.4.6?
* H8/300 was supported for the duration of the full GCC version series, with 3.4.6 [closing the release series](https://gcc.gnu.org/gcc-3.4/changes.html)
* GPC integration is less robust in GCC 4
* Fortran77/g77 support was not included in later GCC versions, and some older code is not well suited to being built by newer Fortran compilers such as `gfortran` ([ref 1](https://github.com/weevington/gcc-3.4.6-41-compat), [ref 2](https://forums.linuxmint.com/viewtopic.php?t=261066)).
* Multiple ongoing patch sources were provided by Linux distributions such as RedHat and Debian
* Creates smaller binaries compared to builds of the same code created using GCC 4
  + This is especially important on memory-constrained devices such as the LEGO MindStorms RCX
  + Example: For the exact same [brickOS-bibo](https://github.com/BrickBot/brickOS-bibo) kernel source code and build configuration, the generated binaries are notably smaller with GCC 3.4.6—

| GCC Version | Kernel Binary File Size (bytes) | App Start (BASE1) Address |
| ----------- | ------------------------------- | ------------------------- |
|  `3.4.6`    |  `13,108`                       |  `0xace4`                 |
|  `4.4.7`    |  `14,322`                       |  `0xb0c4`                 |

Resources for GCC 3.4.6
* [Build and Installation Configuration Documentation](https://web.archive.org/web/20041013092023/https://gcc.gnu.org/install/configure.html)
* [Manual](https://gcc.gnu.org/onlinedocs/gcc-3.4.6/gcc/)
  + [Manual subsets and/or other formats](https://gcc.gnu.org/onlinedocs/) (scroll down for the GCC 3.4.6 manuals section)

### Updates and Modifications
For patches applied from other sources, a more in-depth description is available in the [patches folder](patches/).


### Combined Folder Composition
Several folders are duplicated across the various project comprising the toolchain.
By appropriately mixing and matching folder versions, it is possible to establish
a combined source folder through which a single, combined build can be executed.

Additionally, with two minor tweaks to binutil’s bfd
(adding BFD_HOSTPTR_T and bfd_fopen()),
gdb 6.8 can also be included in this combined build.

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
| boehm-gc     | GCC (libs) | 4.4.7              | 4.4.7                |
| cgen         | binutils   | 2.16.1             | 2.16.1               |
| config       | gcc        | 3.4.6              | 4.4.7                |
| contrib      | gcc        | (n/a)              | 4.4.7                |
| cpu          | binutils   | 2.16.1             | 2.16.1               |
| etc          | binutils   | 2.16.1             | 2.16.1               |
| fastjar      | gcc        | 3.4.6              | (n/a)                |
| fixincludes  | gcc        | (n/a)              | 4.4.7                |
| gas          | binutils   | 2.16.1             | 2.16.1               |
| gcc          | gcc        | 3.4.6              | 4.4.7                |
| gcc/p        | GPC        | 2.1-20070904       | 2.1-20070904         |
| gdb          | gdb        | 6.8                | 6.8                  |
| gnattools    | gdb        | (n/a)              | 6.8                  |
| gprof        | binutils   | 2.16.1             | 2.16.1               |
| include      | GCC (libs) | 4.4.7              | 4.4.7                |
| intl         | GCC (libs) | 4.4.7              | 4.4.7                |
| ld           | binutils   | 2.16.1             | 2.16.1               |
| libada       | gcc        | (n/a)              | 4.4.7                |
| libcpp       | gcc        | (n/a)              | 4.4.7                |
| libdecnumber | GCC (libs) | 4.4.7              | 4.4.7                |
| libf2c       | gcc        | 3.4.6              | (n/a)                |
| libffi       | GCC (libs) | 4.4.7              | 4.4.7                |
| libgcc       | gcc        | (n/a)              | 4.4.7                |
| libgfortran  | gcc        | (n/a)              | 4.4.7                |
| libgloss     | newlib     | 1.19.0             | 1.19.0               |
| libgomp      | gcc        | (n/a)              | 4.4.7                |
| libiberty    | GCC (libs) | 4.4.7              | 4.4.7                |
| libjava      | GCC (libs) | 4.4.7              | 4.4.7                |
| libmudflap   | gcc        | (n/a)              | 4.4.7                |
| libobjc      | gcc        | 3.4.6              | 4.4.7                |
| libstdc++-v3 | gcc        | 3.4.6              | 4.4.7                |
| newlib       | newlib     | 1.19.0             | 1.19.0               |
| opcodes      | binutils   | 2.16.1             | 2.16.1               |
| readline     | gdb        | 6.8                | 6.8                  |
| sim          | gdb        | 6.8                | 6.8                  |
| texinfo      | binutils   | 2.16.1             | 2.16.1               |
| zlib         | GCC (libs) | 4.4.7              | 4.4.7                |

The soft-linked files under that same folder are all from the respective GCC version:

| File              | In GCC 3 | In GCC 4.4 |
| ----------------- | -------- | ---------- |
| Makefile.def      | ×        | ×          |
| Makefile.in       | ×        | ×          |
| Makefile.tpl      | ×        | ×          |
| compile           |          | ×          |
| config-ml.in      | ×        | ×          |
| config.guess      | ×        | ×          |
| config.if         | ×        |            |
| config.rpath      | ×        | ×          |
| config.sub        | ×        | ×          |
| configure         | ×        | ×          |
| configure.[in|ac] | ×        | ×          |
| depcomp           |          | ×          |
| install-sh        | ×        | ×          |
| libtool-ldflags   |          | ×          |
| libtool.m4        | ×        | ×          |
| ltgcc.m4          |          | ×          |
| ltcf-c.sh         | ×        |            |
| ltcf-cxx.sh       | ×        |            |
| ltcf-gcj.sh       | ×        |            |
| ltconfig          | ×        |            |
| ltmain.sh         | ×        | ×          |
| ltoptions.m4      |          | ×          |
| ltsugar.m4        |          | ×          |
| ltversion.m4      |          | ×          |
| missing           | ×        | ×          |
| mkdep             |          | ×          |
| mkinstalldirs     | ×        | ×          |
| move-if-change    | ×        | ×          |
| symlink-tree      | ×        | ×          |
| ylwrap            | ×        | ×          |
