GNU Legacy Toolchain
&nbsp; &nbsp; &nbsp;
[![GNU Legacy Toolchain CI](https://github.com/BrickBot/GNU-Legacy-Toolchain/actions/workflows/gnu-legacy-toolchain_CI.yml/badge.svg)](https://github.com/BrickBot/GNU-Legacy-Toolchain/actions/workflows/gnu-legacy-toolchain_CI.yml)
&nbsp;
![GitHub License](https://img.shields.io/github/license/BrickBot/GNU-Legacy-Toolchain)
====================
A legacy GNU toolchain that includes BinUtils, GCC, GPC, GDB, and NewLib.

Multiple use cases exist for continuing to maintain a legacy toolchain:
* **COFF targets, such as for the H8/300 Processor**
* **GNU Pascal Compiler (GPC)**
* **Fortran77 / G77 Compiler**
* **Common Language Infrastructure (CLI) Common Intermediate Language (CIL) Front End**

While primary goal of this respository is to build a cross-toolchain targeting Hitachi/Renesas H8/300 processors with the COFF format, this should work for other targets as well.
Though this H8/300 target has colloquially been referred to as h8300-hitachi-hms, it is more properly identified as
h8300-hitachi-coff ([ref 1](https://tracker.debian.org/pkg/gcc-h8300-hms), [ref 2](https://sources.debian.org/src/gcc-h8300-hms/1%3A3.4.6%2Bdfsg2-4.2/debian/rules/#L30),
[object file format info](https://maskray.me/blog/2024-01-14-exploring-object-file-formats)).


Quick Start Build Guide
-----------------------
Build instructions are largely the same as those for GCC, with a few additional steps and options.
A working example can be seen in the [Continuous Integration workflow](.github/workflows),
but the basic sequence is as follows:
1. Have a working Linux, Unix, WSL, or Cygwin environment with Bash shell support (currently untested under Mac)
2. Install build dependencies
   1. See the [Continuous Integration workflow](.github/workflows) for a current dependency list
   2. If you platform/distro does not include a package for git-restore-mtime ([like Debian does](https://packages.debian.org/search?keywords=git-restore-mtime&searchon=names&suite=stable&section=all)) _and_ you will be cloning from git, install this manually (c.f. the [git-tools project page](https://github.com/MestreLion/git-tools) for both various distro-specific package names and manual install instructions).
3. Obtain a copy of the source via _either_ of the following means:
   1. Clone from GitHub:  `git clone [--branch <name of branch or tag>] https://github.com/BrickBot/GNU-Legacy-Toolchain.git`  –or–
   2. Download and extract a source archive, such as from one of the [releases](releases)
4. Fix source file timestamps
   1. If you have downloaded the source \*.tar.gz and extracted via a command like `tar --directory=<destination> --extract --file=<source>.tar.gz`, then the embedded timestamps should be adequate and further action in this regard should be necessary.
   2. _However_, if you clone the source from Git or obtain via other similar means, you will need to execute the script `fix-mtime` that is found in the root of the source tree.
   3. **HIGHLY IMPORTANT NOTES**:
	  1. This script could take several minutes to complete, depending on the speed of the system and file access times.
	  2. This will potentially to be rerun after switching branches—if a build unexpectedly fails after switching branches, try rerunning.  
      3. If the disk space is available, consider using `git worktree` instead of switching among Git branches.
5. Create a folder for building that is _outside_ the source tree
6. From that build folder, run either `configure` or one of the following use-case-specific wrapper helpers:
   1. `h8300-hitachi-coff-configure`:  For use with COFF targets for the Hitachi H8/300, which defaults to an integrated toolchain based on GCC v3.
       The target `h8300-hitachi-elf` is additionally enabled, which adds `elf32-h8300` support to tools such as `objcopy`.
   2. `rcx-lego-configure`:  Created for use with the LEGO MindStorms RCX, which defaults to an integrated toolchain based on the above but with a separate GDB v5 instead.
7. Run `make`
8. Run `make install` to copy the files into an installation structure.
   1. By default, the install will be to a separate folder from which [Stow](https://www.gnu.org/software/stow/) can then be used to link the files into your system installation.
   2. In this manner, [Stow](https://www.gnu.org/software/stow/) facilitates a cleaner and easier way to manage installs of locally-built software.
9. Run `make stow` to then link the files into your system installation.

In an explicit, intentional deviation from GNU standards, `install` targets have
been modified to remove `install-info` targets as prerequisites.  The files
installed by these `install-info` targets conflict with other, newer files and
generally only cause problems if included as part of a regular install.  Thus,
these files are excluded (which also simplifies packaging work).

If wishing to install these files, `make install-info` can still be executed separately.


Advanced Options for Configure
------------------------------
Toolchain sets selection: `--toolchain-sets=<comma-separated list of set(s)>`

To allow different toolchain sets to be installed side-by-side,
each toolchain set is configured to be build with a different program suffix.

| Set(s)      | Program Suffix | Description |
| ----------- | -------------- | ----------- |
| binutils216 | `-2.16.1`      | Builds an independent BinUtils only, based on BinUtils 2.16.1 but—like GCC 3—using some common elements from GCC 3.4.6 and GCC 4.4.7<br/> **NOTE**: This is for an independent build of BinUtils ONLY and is **NOT** to be combined with the other toolchain sets. |
| gdb5        | `-5`           | Builds an independent GDB only, based on GDB 5.3 |
| gcc3        | `-3`           | Builds a combined toolchain set based on GCC 3.4.6 but using some common elements from GCC 4.4.7 |
| gcc44       | `-4.4`         | Builds a combined toolchain set based on GCC 4.4.7 |
| gdb5,gcc3   | (_respective_) | Builds a combined toolchain set based on GCC 3.4.6 but uses an independent GDB based on GDB 5.3 |
| gdb5,gcc44  | (_respective_) | Builds a combined toolchain set based on GCC 4.4.7 but uses an independent GDB based on GDB 5.3 |
| gdb5,gcc3,gcc44 | (_respective_) | Builds all three.  If combining into a single install, files conflicts not already resolved by the differing toolchain program suffixes are resolved as follows: GCC 4.4 takes precedence over GCC 3, which takes precedence over GDB 5 |


Repository Composition Notes
----------------------------
The primary selection criteria was the last known versions to include support for h8300-\*-coff,
but this also overlapped well with support for GPC, Fortran77/g77, and the CLI CIL front end.

| Project  | Version  | Release Date | High-Level Notes |
| -------- | -------- | ------------ | ---------------- |
| [Config](https://cgit.git.savannah.gnu.org/cgit/config.git/tree/) | 2024-07-27 | 2024-07-27 | Copies of the latest `config.guess` and `config.sub` files.  **All** config.guess and config.sub files in **all** included source projects are sym-linked to the latest files here. |
| [BinUtils](https://gnu.org/software/binutils/) | 2.16.1   | [2005-06-12](https://sourceware.org/pub/binutils/releases/) ¹ | Note lack of support for h8300-\*-coff in gas/configure.tgt in later versions |
| [GCC](https://gnu.org/software/gcc/)      | 3.4.6    | [2006-03-06](https://gcc.gnu.org/develop.html) ² | Last _full_ version series to support h8300-\*-coff |
| [GCC](https://gnu.org/software/gcc/)      | 4.4.7    | [2012-03-13](https://gcc.gnu.org/develop.html) ² | Support for “Generic COFF” in general was [dropped following the GCC 4.4 release series](https://gcc.gnu.org/gcc-4.4/changes.html).  As the last GCC release to support h8300-\*-coff (c.f. the lack of support for h8300-\*-coff [covered by the "h8300-\*-\*" case] in libgcc/config.host in later versions), the flag `--enable-obsolete` _must_ be used when configuring. |
| [GCC CIL Front End](https://gcc.gnu.org/projects/cli.html) | 4.3.0-2007-12-13 | [2011-06-20](https://gcc.gnu.org/git/?p=gcc.git;a=shortlog;h=refs/vendors/st/heads/cli-fe) (final commit) | Work was done on separate branches, and front end work was later forked off from back end work.  Earlier coding was against GCC 4.3 before later skipping to GCC 4.5 (in which h8300-\*-coff was no longer supported). |
| [GPC](https://www.gnu-pascal.de/gpc/h-index.html)      | 2.1-20070904 | 2007-09-04 | See the links and included README files for further details. ³ |
| [NewLib](https://sourceware.org/newlib/)  | 1.19.0 | [2010-12-16](https://sourceware.org/newlib/) ⁴ | Version 1.20.0 introduces [incompatibilities with libiberty](https://gcc-patches.gcc.gnu.narkive.com/zeSeZ9N8/newlib-vs-libiberty-mismatch-breaks-build-re-patch-export-psignal-on-all-platforms#post1).  Versions 2.0 and later fail to build if targeting h8300-\*-coff <br/> NewLib is required to build libstdc++-v3 for targets such as h8300-hitachi-coff; otherwise, `configure` checks will fail with a “No support for this host/target combination” message. |
| [GDB](https://sourceware.org/gdb/)      | 5.3      | [released 2002-12-12](https://sourceware.org/gdb/schedule/) | While a few later versions still supported h8300-\*-coff targets, the debugging capabilities built into BrickEmu (an emulator for the LEGO MindStorms RCX) are based on a built-in GDB server that implements the GDB 5 protocol. |
| [GDB](https://sourceware.org/gdb/)      | 6.8      | [released 2008-02-29](https://sourceware.org/gdb/schedule/) | This version of GDB best aligns with the other packages in the GCC toolchain, in that it is able to be included in a combined toolchain build.  (Neither GDB 5.3 nor GDB 7.12.1 are able to be built that way.) |
| [GDB](https://sourceware.org/gdb/)      | 7.12.1   | [released 2017-01-21](https://sourceware.org/gdb/schedule/) | While GDB 7.12.1 seems to indicate that h8300-\*-\*-coff targets are supported (note lack of support for h8300-\*-\*-coff [covered by the "h8300-\*-\*-\*" case] in bfd/config.bfd in later versions), compatibility has not been fully validated, as it was released well after h8300-\*-coff support had been dropped from other packages (for example, BinUtils and GDB are both developed in a single, common source repository). |

 ¹ BinUtils:  Release date based on the ChangeLog files for bfd, gas, and ld.

 ² GCC:  Link includes the full GCC release timeline, with additional resources as follows:
  + [Build and Installation Configuration Documentation for GCC 3.4.6](https://web.archive.org/web/20041013092023/https://gcc.gnu.org/install/configure.html)
  + [GCC 3.4.6 Manual](https://gcc.gnu.org/onlinedocs/gcc-3.4.6/gcc/)
  + [GCC 4.4.7 Manual](https://gcc.gnu.org/onlinedocs/gcc-4.4.7/gcc/)
  + [Manual subsets and/or other formats](https://gcc.gnu.org/onlinedocs/) (scroll down for the GCC 3.4.6 and 4.4.7 manuals section)
  + The Ada programming language apparently does not support bootstrapping, so it is not currently available with either GCC v3 or GCC v4.4.

 ³ GPC:  Additional links and information—
  + [The GNU Pascal Manual](https://www.gnu-pascal.de/gpc/)
  + [GPC website](https://www.gnu-pascal.de/gpc/h-index.html)
    - [Compilation and installation guide](https://www.gnu-pascal.de/gpc/Compiling-GPC.html#Compiling-GPC):  Note the section covering `pascal.install` at the end
    - [Cross-compilation guide](https://www.gnu-pascal.de/gpc/Cross_002dCompilers.html#Cross_002dCompilers)
  + Source files from [hebisch/gpc](https://github.com/hebisch/gpc)
  + [Mailing list](https://www.gnu.de/mailman3/hyperkitty/list/gpc@gnu.de/latest) (no longer seems to be active)
    - [Subscribe/Unsubscribe page](https://www.gnu.de/mailman3/postorius/lists/gpc.gnu.de/)

 ⁴ NewLib:  Release history link opens in a subframe; at the site,
     navigate to [Download](https://sourceware.org/newlib/download.html) > [Snapshots](https://sourceware.org/ftp/newlib/index.html) to view the release timeline history subframe.



### GCC 3.4.6 versus GCC 4.4.7
Use of GCC 3.4.6 generally seems preferable to GCC 4.4.7:
* H8/300 was supported for the duration of the full GCC version series, with 3.4.6 [closing the release series](https://gcc.gnu.org/gcc-3.4/changes.html)
* More programming languages supported
  + GPC integration is less robust in GCC 4
  + Fortran77/g77 support was not included in later GCC versions, and some older code is not well suited to being built by newer Fortran compilers such as `gfortran` ([ref 1](https://github.com/weevington/gcc-3.4.6-41-compat), [ref 2](https://forums.linuxmint.com/viewtopic.php?t=261066)).
* Multiple ongoing patch sources were provided by Linux distributions such as RedHat and Debian
* Creates smaller binaries compared to builds of the same code created using GCC 4
  + This is especially important on memory-constrained devices such as the LEGO MindStorms RCX
  + Example: For the exact same [brickOS-bibo](https://github.com/BrickBot/brickOS-bibo) kernel source code and build configuration, the generated binaries are notably smaller with GCC 3.4.6—

| GCC Version | Supported Programming Languages                       | Kernel Binary File Size | App Start (BASE1) Address |
| ----------- | ----------------------------------------------------- | ----------------------- | ------------------------- |
|  `3.4.6`    | C, C++, Java, ObjectiveC, Fortran77, Pascal, TreeLang |  `13,108` bytes         |  `0xace4`                 |
|  `4.4.7`    | C, C++, Java, CLI CLI front end, possibly Pascal (?)  |  `14,322` bytes         |  `0xb0c4`                 |


Potential advantages of GCC 4.4.7 over GCC 3.4.6:
* A more complete backporting of multiarch was possible for GCC 4.4.7 compared to GCC 3.4.6.
* Common Language Infrastructure (CLI) Common Intermediate Language (CIL) Front End support is only available in GCC 4.4.7.


Additional Details and Information
----------------------------------

### Updates and Modifications
For patches and updates applied from other sources, a more in-depth description is available in the [`patches`](patches/) folder.


### Change Log for Included Source Projects
Changes to each of the source projects as compared to their last official releases can be reviewed as follows:
* [Config](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/config)
  + Currently still able to use the latest config.guess and config.sub from upstream.
  + All config.guess and config.sub files in all included source projects are sym-linked to the files here.
* [BinUtils 2.16.1](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/binutils-2.16.1)
* [GCC 3.4.6](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gcc-3.4.6)
* [GCC 4.4.7](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gcc-4.4.7)
* [GPC 2.1-20070904](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gpc-2.1-20070904)
  + [Short, earlier history](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gpc-master):  No substantive changes here, but included for completeness
* [NewLib 1.19.0](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/newlib-1.19.0)
* [GDB 5.3](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gdb-5.3)
* [GDB 6.8](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gdb-6.8)
* [GDB 7.12.1](https://github.com/BrickBot/GNU-Legacy-Toolchain/commits/master/src-projects/gdb-7.12.1):  Not currently used in this project, but included for reference


### Compostion of Combined Folders
Several folders are duplicated across the various project comprising the toolchain.
By appropriately mixing and matching folder versions, it is possible to establish
a combined source folder through which a single, combined build can be executed.

More information on how projects were “mixed and matched” to create combined
source folders is available in the [`sym-combined`](sym-combined/) folder.
