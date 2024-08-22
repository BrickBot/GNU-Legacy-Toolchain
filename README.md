H8/300 COFF (HMS) Toolchain
===========================
A cross-toolchain targeting Hitachi H8/300 processors with the COFF (or, colloquially, HMS) format, including binutils, gcc, gpc, gdb, and newlib.


Repository Creation Notes
-------------------------

### Sources

Why GCC 3.4.6?
* H8/300 was supported for the duration of the full GCC version series
* Source code has been suppored by Debian package maintainers with maintance patches (gcc-h8300 package)
* GPC integration is less robust in GCC 4
* Created smaller binaries compared to the same builds created using GCC 4
  + Especially important on memory-constrained devices such as the LEGO MindStorms RCX


### Combined Folder Composition
The following were soft-linked in to create the combined source folder:

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

Folders from GDB
* gdb
* libdecnumber
* readline
* sim

Folders from NewLib
* libgloss
* newlib

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


### Patches
The following patches were applied.

#### Binutils
Available Debian patches include those lised below (from the Debian source archive patches/series file).
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
Available Debian patches include those lised below (from the Debian source archive patches/series file).
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
