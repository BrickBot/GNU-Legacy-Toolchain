#!/bin/sh
sed -i -e 's/struct siginfo/siginfo_t/' gcc/config/*/linux*.h
