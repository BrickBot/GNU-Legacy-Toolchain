
# Copyright (C) 2007 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#
# Authors:
#    Andrea Ornstein
#    Erven Rohou
#    Roberto Costa
#
# Contact information at STMicroelectronics:
# Andrea C. Ornstein    <andrea.ornstein@st.com>
# Erven Rohou           <erven.rohou@st.com>

ACLOCAL_AMFLAGS = -I .. -I ../config

# May be used by various substitution variables.
gcc_version := $(shell cat $(top_srcdir)/../gcc/BASE-VER)

toolexeclib_LTLIBRARIES = libcilsupport.la

libcilsupport_la_SOURCES = \
	gcc4net-support.c \
	libstd-support.c \
	runtime-support.c

# No install-html or install-pdf support in automake yet
.PHONY: install-html install-pdf
install-html:

install-pdf:
