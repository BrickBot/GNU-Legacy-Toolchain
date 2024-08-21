H8/300 COFF (HMS) Toolchain
===========================
A cross-toolchain targeting Hitachi H8/300 processors with the COFF (or, colloquially, HMS) format, including binutils, gcc, gpc, gdb, and newlib.


Repository Creation Notes
-------------------------

### Sources


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
