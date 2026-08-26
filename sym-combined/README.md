Compostion of Combined Folders
==============================
Several folders are duplicated across the various project comprising the toolchain.
By appropriately mixing and matching folder versions, it is possible to establish
a combined source folder through which a single, combined build can be executed.

Additionally, by implementing two minor tweaks to binutil’s bfd
(adding BFD_HOSTPTR_T and bfd_fopen()),
GDB 6.8 can also be included in combined builds.


Mapping Folders Common/Shared across Projects
---------------------------------------------

| Folder       | gcc 3.4.6 | gcc 4.4.7 | binutils 2.16.1 | newlib 1.19.0 | gdb 6.8 |
| ------------ | --------- | --------- | --------------- | ------------- | ------- |
| bfd          |           |           |  ×              |               |  ×      |
| cpu          |           |           |  ×              |               |  ×      |
| etc          |           |           |  ×              |  ×            |  ×      |
| include      |  ×        |  ×        |  ×              |               |  ×      |
| intl         |  ×        |  ×        |  ×              |               |  ×      |
| libdecnumber |           |  ×        |                 |               |  ×      |
| libiberty    |  ×        |  ×        |  ×              |               |  ×      |
| opcodes      |           |           |  ×              |               |  ×      |
| texinfo      |           |           |  ×              |  ×            |  ×      |
| zlib         |  ×        |  ×        |                 |               |  ×      |



Creating Folders for Combined Builds
------------------------------------
The following folders were sym-linked in to create the corresponding directories
under the combined source folder (`sym-combined`),
which is then used as the source folder for builds.
(Note in particular that the GCC libs from GCC 4.4.7 are used with the combined GCC 3 build.)

| Folder       | Source     | BinUtils2.16.1-Based Version | GCC3-Based Version | GCC4.4-Based Version |
| ------------ | ---------- | ---------------------------- | ------------------ | -------------------- |
| bfd          | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| binutils     | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| boehm-gc     | GCC (libs) | —                            | 4.4.7              | 4.4.7                |
| cgen         | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| config       | gcc        | 3.4.6                        | 3.4.6              | 4.4.7                |
| contrib      | gcc        | —                            | —                  | 4.4.7                |
| cpu          | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| etc          | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| fastjar      | gcc        | —                            | 3.4.6              | —                    |
| fixincludes  | gcc        | —                            | —                  | 4.4.7                |
| gas          | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| gcc          | gcc        | —                            | 3.4.6              | 4.4.7                |
| gcc/p        | GPC        | —                            | 2.1-20070904       | 2.1-20070904         |
| gdb          | gdb        | —                            | 5.3                | 6.8                  |
| gnattools    | gdb        | —                            | —                  | 6.8                  |
| gprof        | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| include      | GCC (libs) | 4.4.7                        | 4.4.7              | 4.4.7                |
| intl         | GCC (libs) | 4.4.7                        | 4.4.7              | 4.4.7                |
| ld           | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| libada       | gcc        | —                            | —                  | 4.4.7                |
| libcpp       | gcc        | —                            | —                  | 4.4.7                |
| libdecnumber | GCC (libs) | —                            | 4.4.7              | 4.4.7                |
| libf2c       | gcc        | —                            | 3.4.6              | —                    |
| libffi       | GCC (libs) | —                            | 4.4.7              | 4.4.7                |
| libgcc       | gcc        | —                            | —                  | 4.4.7                |
| libgfortran  | gcc        | —                            | —                  | 4.4.7                |
| libgloss     | newlib     | —                            | 1.19.0             | 1.19.0               |
| libgomp      | gcc        | —                            | —                  | 4.4.7                |
| libiberty    | GCC (libs) | 4.4.7                        | 4.4.7              | 4.4.7                |
| libjava      | GCC (libs) | —                            | 4.4.7              | 4.4.7                |
| libmudflap   | gcc        | —                            | —                  | 4.4.7                |
| libobjc      | gcc        | —                            | 3.4.6              | 4.4.7                |
| libstdc++-v3 | gcc        | —                            | 3.4.6              | 4.4.7                |
| newlib       | newlib     | —                            | 1.19.0             | 1.19.0               |
| opcodes      | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| readline     | gdb        | —                            | 6.8                | 6.8                  |
| sim          | gdb        | —                            | 5.3                | 6.8                  |
| texinfo      | binutils   | 2.16.1                       | 2.16.1             | 2.16.1               |
| zlib         | GCC (libs) | —                            | 4.4.7              | 4.4.7                |


The sym-linked files under that same folder are each from the respective GCC version:

| File               | In BinUtils 2.16.1 from GCC 3 | In GCC 3 | In GCC 4.4 |
| ------------------ | ----------------------------- | -------- | ---------- |
| Makefile.def       | ×                             | ×        | ×          |
| Makefile.in        | ×                             | ×        | ×          |
| Makefile.tpl       | ×                             | ×        | ×          |
| compile            |                               |          | ×          |
| config-ml.in       | ×                             | ×        | ×          |
| config.guess       | Config                        | Config   | Config     |
| config.if          | ×                             | ×        |            |
| config.rpath       |                               | ×        | ×          |
| config.sub         | Config                        | Config   | Config     |
| configure          | ×                             | ×        | ×          |
| configure.[in\|ac] | ×                             | ×        | ×          |
| depcomp            |                               |          | ×          |
| gettext.m4         | BinUtils                      | BinUtils | BinUtils   |
| install-sh         | ×                             | ×        | ×          |
| libtool-ldflags    |                               |          | ×          |
| libtool.m4         | ×                             | ×        | ×          |
| ltgcc.m4           |                               |          | ×          |
| ltcf-c.sh          | ×                             | ×        |            |
| ltcf-cxx.sh        | ×                             | ×        |            |
| ltcf-gcj.sh        | ×                             | ×        |            |
| ltconfig           | ×                             | ×        |            |
| ltmain.sh          | ×                             | ×        | ×          |
| ltoptions.m4       |                               |          | ×          |
| ltsugar.m4         |                               |          | ×          |
| ltversion.m4       |                               |          | ×          |
| missing            | ×                             | ×        | ×          |
| mkdep              | ×                             | ×        | ×          |
| mkinstalldirs      | ×                             | ×        | ×          |
| move-if-change     | ×                             | ×        | ×          |
| symlink-tree       | ×                             | ×        | ×          |
| ylwrap             | ×                             | ×        | ×          |
