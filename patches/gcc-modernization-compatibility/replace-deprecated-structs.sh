#!/bin/sh
# Tip: Before running, verify with ‘grep -n -r -F "struct siginfo"’ and ‘grep -n -r -F "struct ucontext"’

sed -i -e 's/struct siginfo/siginfo_t/g'   gcc/config/*/linux*.h  boehm-gc/os_dep.c

sed -i -e 's/struct ucontext/ucontext_t/g' gcc/config/*/linux*.h  libjava/include/*-signal.h  gcc/testsuite/gcc.target/sparc/*getcontext*.c
