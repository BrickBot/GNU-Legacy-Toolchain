%global DATE 20120601
%global SVNREV 188105
# Note, gcc_release must be integer, if you want to add suffixes to
# %{release}, append them after %{gcc_release} on Release: line.
%global gcc_release 8
%global _unpackaged_files_terminate_build 0
%global multilib_64_archs sparc64 ppc64 s390x x86_64
%ifarch s390x
%global multilib_32_arch s390
%endif
%ifarch sparc64
%global multilib_32_arch sparcv9
%endif
%ifarch ppc64
%global multilib_32_arch ppc
%endif
%ifarch x86_64
%global multilib_32_arch i686
%endif
Summary: Compatibility GNU Compiler Collection
Name: compat-gcc-44
Version: 4.4.7
Release: %{gcc_release}%{?dist}
# libgcc, libgfortran, libmudflap, libgomp, libstdc++ and crtstuff have
# GCC Runtime Exception.
License: GPLv3+ and GPLv3+ with exceptions and GPLv2+ with exceptions
Group: Development/Languages
# The source for this package was pulled from upstream's vcs.  Use the
# following commands to generate the tarball:
# svn export svn://gcc.gnu.org/svn/gcc/branches/redhat/gcc-4_4-branch@%{SVNREV} gcc-%{version}-%{DATE}
# tar cf - gcc-%{version}-%{DATE} | bzip2 -9 > gcc-%{version}-%{DATE}.tar.bz2
Source0: gcc-%{version}-%{DATE}.tar.bz2
Source1: dummylib.sh
URL: http://gcc.gnu.org
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
# Need binutils with -pie support >= 2.14.90.0.4-4
# Need binutils which can omit dot symbols and overlap .opd on ppc64 >= 2.15.91.0.2-4
# Need binutils which handle -msecure-plt on ppc >= 2.16.91.0.2-2
# Need binutils which support .weakref >= 2.16.91.0.3-1
# Need binutils which support --hash-style=gnu >= 2.17.50.0.2-7
# Need binutils which support mffgpr and mftgpr >= 2.17.50.0.2-8
# Need binutils which support --build-id >= 2.17.50.0.17-3
# Need binutils which support %gnu_unique_object >= 2.19.51.0.14
# Need binutils which support .cfi_sections >= 2.19.51.0.14-33
# Need binutils which support --no-add-needed >= 2.20.51.0.2-12
BuildRequires: binutils >= 2.20.51.0.2-12
# While gcc doesn't include statically linked binaries, during testing
# -static is used several times.
BuildRequires: glibc-static
BuildRequires: zlib-devel, gettext, dejagnu, bison, flex, texinfo, sharutils
# For VTA guality testing
BuildRequires: gdb
# Make sure pthread.h doesn't contain __thread tokens
# Make sure glibc supports stack protector
# Make sure glibc supports DT_GNU_HASH
BuildRequires: glibc-devel >= 2.4.90-13
BuildRequires: elfutils-devel >= 0.72
%ifarch ppc ppc64 s390 s390x sparc sparcv9 alpha
# Make sure glibc supports TFmode long double
BuildRequires: glibc >= 2.3.90-35
%endif
%ifarch %{multilib_64_archs} sparcv9 ppc
# Ensure glibc{,-devel} is installed for both multilib arches
BuildRequires: /lib/libc.so.6 /usr/lib/libc.so /lib64/libc.so.6 /usr/lib64/libc.so
%endif
%ifarch ia64
BuildRequires: libunwind >= 0.98
%endif
# Need .eh_frame ld optimizations
# Need proper visibility support
# Need -pie support
# Need --as-needed/--no-as-needed support
# On ppc64, need omit dot symbols support and --non-overlapping-opd
# Need binutils that owns /usr/bin/c++filt
# Need binutils that support .weakref
# Need binutils that supports --hash-style=gnu
# Need binutils that support mffgpr/mftgpr
# Need binutils that support --build-id
# Need binutils that support %gnu_unique_object
# Need binutils that support .cfi_sections
# Need binutils that support --no-add-needed
Requires: binutils >= 2.20.51.0.2-12
# Make sure gdb will understand DW_FORM_strp
Conflicts: gdb < 5.1-2
Requires: glibc-devel >= 2.2.90-12
%ifarch ppc ppc64 s390 s390x sparc sparcv9 alpha
# Make sure glibc supports TFmode long double
Requires: glibc >= 2.3.90-35
%endif
Requires: libgcc >= %{version}-%{release}
Requires: libgomp >= %{version}-%{release}
AutoReq: true
AutoProv: false
Obsoletes: compat-egcs
Obsoletes: compat-gcc
Obsoletes: compat-gcc-objc
Obsoletes: compat-egcs-objc
Obsoletes: compat-gcc-g77
Obsoletes: compat-egcs-g77
Obsoletes: compat-gcc-java
Obsoletes: compat-libgcj
Obsoletes: compat-libgcj-devel
Obsoletes: compat-gcc-32
Obsoletes: compat-gcc-34
Obsoletes: gcc44

Patch0: gcc44-hack.patch
Patch1: gcc44-build-id.patch
Patch2: gcc44-c++-builtin-redecl.patch
Patch3: gcc44-ia64-libunwind.patch
Patch4: gcc44-java-nomulti.patch
Patch5: gcc44-ppc32-retaddr.patch
Patch6: gcc44-pr33763.patch
Patch7: gcc44-rh330771.patch
Patch8: gcc44-i386-libgomp.patch
Patch9: gcc44-sparc-config-detection.patch
Patch10: gcc44-libgomp-omp_h-multilib.patch
Patch11: gcc44-libtool-no-rpath.patch
Patch12: gcc44-cloog-dl.patch
Patch13: gcc44-unwind-debug-hook.patch
Patch14: gcc44-pr38757.patch
Patch15: gcc44-libstdc++-docs.patch
Patch16: gcc44-ppc64-aixdesc.patch
Patch17: gcc44-no-add-needed.patch
Patch18: gcc44-rh610785.patch
Patch19: gcc44-rh533181.patch
Patch20: gcc44-pr48857-test.patch
Patch21: gcc44-pr43680.patch
Patch22: gcc44-rh750545.patch
Patch23: gcc44-pr53199.patch
Patch24: gcc44-rh801144.patch
Patch25: gcc44-rh808590.patch
Patch26: gcc44-rh820281.patch
Patch27: gcc44-pr54858.patch
Patch28: gcc44-pr54487.patch
Patch29: gcc44-rh867878.patch
Patch30: gcc44-pr49146.patch
Patch31: gcc44-pr56403.patch
Patch32: gcc44-rh906234.patch
Patch33: gcc44-rh908025.patch
Patch34: gcc44-rh967003.patch

Patch100: gcc44-texinfo.patch
Patch101: gcc44-siginfo_t.patch

# On ARM EABI systems, we do want -gnueabi to be part of the
# target triple.
%ifnarch %{arm}
%global _gnu %{nil}
%endif
%ifarch sparcv9
%global gcc_target_platform sparc64-%{_vendor}-%{_target_os}
%endif
%ifarch ppc
%global gcc_target_platform ppc64-%{_vendor}-%{_target_os}
%endif
%ifnarch sparcv9 ppc
%global gcc_target_platform %{_target_platform}
%endif

%description
This package includes a GCC 4.4.7-RH compatibility compiler.

%package c++
Summary: C++ support for compatibility compiler
Group: Development/Languages
Requires: compat-gcc-44 = %{version}-%{release}
Requires: libstdc++ >= 4.8.0, libstdc++ < 4.9.0
BuildRequires: libstdc++
Autoreq: true
Autoprov: false
Obsoletes: compat-egcs-c++
Obsoletes: compat-gcc-c++
Obsoletes: compat-libstdc++
Obsoletes: compat-libstdc++-devel
Obsoletes: compat-gcc-32-c++
Obsoletes: compat-gcc-34-c++
Obsoletes: gcc44-c++

%description c++
This package includes a GCC 4.4.7-RH compatibility C++ compiler.

%package gfortran
Summary: Fortran support for compatibility compiler
Group: Development/Languages
Requires: compat-gcc-44 = %{version}-%{release}
Requires: libgfortran >= 4.8.0, libgfortran < 4.9.0
BuildRequires: gmp-devel >= 4.1.2-8, mpfr-devel >= 2.2.1
BuildRequires: libgfortran
Autoreq: true
Autoprov: false
Obsoletes: gcc3-g77
Obsoletes: gcc-g77
Obsoletes: compat-gcc-32-g77
Obsoletes: compat-gcc-34-g77
Obsoletes: gcc44-gfortran

%description gfortran
This package includes a GCC 4.4.7-RH compatibility Fortran compiler.

%prep
%setup -q -n gcc-%{version}-%{DATE}
%patch0 -p0 -b .hack~
%patch1 -p0 -b .build-id~
%patch2 -p0 -b .c++-builtin-redecl~
%patch3 -p0 -b .ia64-libunwind~
%patch4 -p0 -b .java-nomulti~
%patch5 -p0 -b .ppc32-retaddr~
%patch6 -p0 -b .pr33763~
%patch7 -p0 -b .rh330771~
%patch8 -p0 -b .i386-libgomp~
%patch9 -p0 -b .sparc-config-detection~
%patch10 -p0 -b .libgomp-omp_h-multilib~
%patch11 -p0 -b .libtool-no-rpath~
%patch13 -p0 -b .unwind-debug-hook~
%patch14 -p0 -b .pr38757~
%patch16 -p0 -b .ppc64-aixdesc~
%patch17 -p0 -b .no-add-needed~
%patch18 -p0 -b .rh610785~
%patch19 -p0 -b .rh533181~
%patch20 -p0 -b .pr48857-test~
%patch21 -p0 -b .pr43680~
%patch22 -p0 -b .rh750545~
%patch23 -p0 -b .pr53199~
%patch24 -p0 -b .rh801144~
%patch25 -p0 -b .rh808590~
%patch26 -p0 -b .rh820281~
%patch27 -p0 -b .pr54858~
%patch28 -p0 -b .pr54487~
%patch29 -p0 -b .rh867878~
%patch30 -p0 -b .pr49146~
%patch31 -p0 -b .pr56403~
%patch32 -p0 -b .rh906234~
%patch33 -p0 -b .rh908025~
%patch34 -p0 -b .rh967003~

%patch100 -p0 -b .texinfo~
%patch101 -p0 -b .siginfo_t~

#sed -i -e 's/4\.4\.7/4.4.7/' gcc/BASE-VER
echo 'Red Hat %{version}-%{gcc_release}' > gcc/DEV-PHASE

# Default to -gdwarf-3 rather than -gdwarf-2
sed -i '/UInteger Var(dwarf_version)/s/Init(2)/Init(3)/' gcc/common.opt
sed -i 's/\(may be either 2 or 3; the default version is \)2\./\13./' gcc/doc/invoke.texi

cp -a libstdc++-v3/config/cpu/i{4,3}86/atomicity.h

./contrib/gcc_update --touch

LC_ALL=C sed -i -e 's/\xa0/ /' gcc/doc/options.texi

%ifarch ppc
if [ -d libstdc++-v3/config/abi/post/powerpc64-linux-gnu ]; then
  mkdir -p libstdc++-v3/config/abi/post/powerpc64-linux-gnu/64
  mv libstdc++-v3/config/abi/post/powerpc64-linux-gnu/{,64/}baseline_symbols.txt
  mv libstdc++-v3/config/abi/post/powerpc64-linux-gnu/{32/,}baseline_symbols.txt
  rm -rf libstdc++-v3/config/abi/post/powerpc64-linux-gnu/32
fi
%endif
%ifarch sparc
if [ -d libstdc++-v3/config/abi/post/sparc64-linux-gnu ]; then
  mkdir -p libstdc++-v3/config/abi/post/sparc64-linux-gnu/64
  mv libstdc++-v3/config/abi/post/sparc64-linux-gnu/{,64/}baseline_symbols.txt
  mv libstdc++-v3/config/abi/post/sparc64-linux-gnu/{32/,}baseline_symbols.txt
  rm -rf libstdc++-v3/config/abi/post/sparc64-linux-gnu/32
fi
%endif

%build

rm -fr obj-%{gcc_target_platform}
mkdir obj-%{gcc_target_platform}
cd obj-%{gcc_target_platform}

CC=gcc
OPT_FLAGS=`echo %{optflags}|sed -e 's/\(-Wp,\)\?-D_FORTIFY_SOURCE=[12]//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-fstack-protector-strong/-fstack-protector/g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-m64//g;s/-m32//g;s/-m31//g'`
%ifarch sparc
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mcpu=ultrasparc/-mtune=ultrasparc/g;s/-mcpu=v[78]//g'`
%endif
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-grecord-gcc-switches//g'`
%ifarch %{ix86}
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-march=i.86//g;s/-march=x86_64//g;s/-mfpmath=sse//g'`
%endif
%ifarch ppc ppc64
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-march=power[4678]//g;s/-mtune=power[678]//g'`
%endif
%ifarch s390 s390x
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-march=z10//g;s/-march=z196//g;s/-mtune=zEC12//g'`
%endif
%ifarch sparc64
cat > gcc64 <<"EOF"
#!/bin/sh
exec /usr/bin/gcc -m64 "$@"
EOF
chmod +x gcc64
CC=`pwd`/gcc64
%endif
%ifarch ppc64
if gcc -m64 -xc -S /dev/null -o - > /dev/null 2>&1; then
  cat > gcc64 <<"EOF"
#!/bin/sh
exec /usr/bin/gcc -m64 "$@"
EOF
  chmod +x gcc64
  CC=`pwd`/gcc64
fi
%endif
OPT_FLAGS=`echo "$OPT_FLAGS" | sed -e 's/[[:blank:]]\+/ /g'`
case "$OPT_FLAGS" in
  *-fasynchronous-unwind-tables*)
    sed -i -e 's/-fno-exceptions /-fno-exceptions -fno-asynchronous-unwind-tables/' \
      ../gcc/Makefile.in
    ;;
esac
CC="$CC" CFLAGS="$OPT_FLAGS" CXXFLAGS="`echo $OPT_FLAGS | sed 's/ -Wall / /g'`" XCFLAGS="$OPT_FLAGS" TCFLAGS="$OPT_FLAGS" \
	GCJFLAGS="$OPT_FLAGS" \
	../configure --prefix=%{_prefix} --mandir=%{_mandir} --infodir=%{_infodir} \
	--with-bugurl=http://bugzilla.redhat.com/bugzilla --enable-bootstrap \
	--enable-shared --enable-threads=posix --enable-checking=release \
	--with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions \
	--enable-gnu-unique-object --enable-languages=c,c++,fortran --disable-libgcj \
	--without-cloog --without-ppl \
%ifarch %{arm}
	--disable-sjlj-exceptions \
%endif
%ifarch ppc ppc64
	--enable-secureplt \
%endif
%ifarch sparc sparcv9 sparc64 ppc ppc64 s390 s390x alpha
	--with-long-double-128 \
%endif
%ifarch sparc
	--disable-linux-futex \
%endif
%ifarch sparc64
	--with-cpu=ultrasparc \
%endif
%ifarch sparc sparcv9
	--host=%{gcc_target_platform} --build=%{gcc_target_platform} --target=%{gcc_target_platform} --with-cpu=v7
%endif
%if 0%{?rhel} >= 6
%ifarch ppc ppc64
	--with-cpu-32=power4 --with-tune-32=power6 --with-cpu-64=power4 --with-tune-64=power6 \
%endif
%endif
%ifarch ppc
	--build=%{gcc_target_platform} --target=%{gcc_target_platform} --with-cpu=default32
%endif
%ifarch %{ix86} x86_64
	--with-tune=generic \
%endif
%ifarch %{ix86}
	--with-arch=i686 \
%endif
%ifarch x86_64
	--with-arch_32=i686 \
%endif
%ifarch s390 s390x
	--with-arch=z9-109 --with-tune=z10 --enable-decimal-float \
%endif
%ifnarch sparc sparcv9 ppc
	--build=%{gcc_target_platform}
%endif

#GCJFLAGS="$OPT_FLAGS" make %{?_smp_mflags} BOOT_CFLAGS="$OPT_FLAGS" bootstrap
GCJFLAGS="$OPT_FLAGS" make %{?_smp_mflags} BOOT_CFLAGS="$OPT_FLAGS" profiledbootstrap

# Make generated man pages even if Pod::Man is not new enough
perl -pi -e 's/head3/head2/' ../contrib/texi2pod.pl
for i in ../gcc/doc/*.texi; do
  cp -a $i $i.orig; sed 's/ftable/table/' $i.orig > $i
done
make -C gcc generated-manpages
for i in ../gcc/doc/*.texi; do mv -f $i.orig $i; done

# Make generated doxygen pages.

# Copy various doc files here and there
cd ..
mkdir -p rpm.doc/gfortran
mkdir -p rpm.doc/changelogs/{gcc/cp,libstdc++-v3,libmudflap,libgomp}

for i in {gcc,gcc/cp,libstdc++-v3,libmudflap,libgomp}/ChangeLog*; do
	cp -p $i rpm.doc/changelogs/$i
done

(cd gcc/fortran; for i in ChangeLog*; do
	cp -p $i ../../rpm.doc/gfortran/$i
done)
(cd libgfortran; for i in ChangeLog*; do
	cp -p $i ../rpm.doc/gfortran/$i.libgfortran
done)

rm -f rpm.doc/changelogs/gcc/ChangeLog.[1-9]
find rpm.doc -name \*ChangeLog\* | xargs bzip2 -9

cd -

# Fix up libstdc++.so's
d_first=yes
for d in `pwd`/%{gcc_target_platform}/libstdc++-v3 `pwd`/%{gcc_target_platform}/*/libstdc++-v3; do
  test -d $d || continue
  pushd $d/src
    sh %{SOURCE1} .libs/libstdc++.so .libs/ll.so libstdc++-symbols.ver
    rm .libs/libstdc++.so; cp .libs/ll.so .libs/libstdc++.so
    if [ x"$d_first" = xyes ]; then
      rm .libs/libstdc++.so.6
      libstdcxx_so=`basename %{_prefix}/%{_lib}/libstdc++.so.6.0.*`
      cp -a %{_prefix}/%{_lib}/$libstdcxx_so .libs/
      cd .libs; ln -sf $libstdcxx_so libstdc++.so.6; cd -
      d_first=no
    fi
  popd
done
# Fix up libgomp.so's
d_first=yes
for d in `pwd`/%{gcc_target_platform}/libgomp `pwd`/%{gcc_target_platform}/*/libgomp; do
  test -d $d || continue
  mapf=`pwd`/../libgomp/libgomp.map
  pushd $d
    sh %{SOURCE1} .libs/libgomp.so .libs/ll.so $mapf
    rm .libs/libgomp.so; cp .libs/ll.so .libs/libgomp.so
    if [ x"$d_first" = xyes ]; then
      rm .libs/libgomp.so.1
      libgomp_so=`basename %{_prefix}/%{_lib}/libgomp.so.1.0.*`
      cp -a %{_prefix}/%{_lib}/$libgomp_so .libs/
      cd .libs; ln -sf $libgomp_so libgomp.so.1; cd -
      d_first=no
    fi
  popd
done
# Fix up libgfortran.so's
d_first=yes
for d in `pwd`/%{gcc_target_platform}/libgfortran `pwd`/%{gcc_target_platform}/*/libgfortran; do
  test -d $d || continue
  mapf=`pwd`/../libgfortran/gfortran.map
  pushd $d
    sh %{SOURCE1} .libs/libgfortran.so .libs/ll.so $mapf
    rm .libs/libgfortran.so; cp .libs/ll.so .libs/libgfortran.so
    if [ x"$d_first" = xyes ]; then
      rm .libs/libgfortran.so.3
      libgfortran_so=`basename %{_prefix}/%{_lib}/libgfortran.so.3.0.*`
      cp -a %{_prefix}/%{_lib}/$libgfortran_so .libs/
      cd .libs; ln -sf $libgfortran_so libgfortran.so.3; cd -
      d_first=no
    fi
  popd
done
# Fix up libgcc_s.so's
d_first=yes
for d in `pwd`/%{gcc_target_platform}/libgcc `pwd`/%{gcc_target_platform}/*/libgcc/*/; do
  test -d $d || continue
  pushd $d
    mapf=libgcc.map
    if [ ! -f $mapf ]; then mapf=../libgcc.map; fi
    sh %{SOURCE1} libgcc_s.so ll.so $mapf
    rm libgcc_s.so; cp ll.so libgcc_s.so
    if [ x"$d_first" = xyes ]; then
      rm libgcc_s.so.1
      libgcc_s_so=`basename /%{_lib}/libgcc_s-4*.so.1`
      cp -a /%{_lib}/$libgcc_s_so .
      ln -sf $libgcc_s_so libgcc_s.so.1
      d_first=no
    fi
  popd
done

%install
rm -fr %{buildroot}

cd obj-%{gcc_target_platform}

TARGET_PLATFORM=%{gcc_target_platform}

# There are some MP bugs in libstdc++ Makefiles
make -C %{gcc_target_platform}/libstdc++-v3

make prefix=%{buildroot}%{_prefix} mandir=%{buildroot}%{_mandir} \
  infodir=%{buildroot}%{_infodir} install

FULLPATH=%{buildroot}%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
FULLEPATH=%{buildroot}%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}

# fix some things
gzip -9 %{buildroot}%{_infodir}/*.info*

cxxconfig="`find %{gcc_target_platform}/libstdc++-v3/include -name c++config.h`"
for i in `find %{gcc_target_platform}/[36]*/libstdc++-v3/include -name c++config.h 2>/dev/null`; do
  if ! diff -up $cxxconfig $i; then
    cat > %{buildroot}%{_prefix}/include/c++/%{version}/%{gcc_target_platform}/bits/c++config.h <<EOF
#ifndef _CPP_CPPCONFIG_WRAPPER
#define _CPP_CPPCONFIG_WRAPPER 1
#include <bits/wordsize.h>
#if __WORDSIZE == 32
%ifarch %{multilib_64_archs}
`cat $(find %{gcc_target_platform}/32/libstdc++-v3/include -name c++config.h)`
%else
`cat $(find %{gcc_target_platform}/libstdc++-v3/include -name c++config.h)`
%endif
#else
%ifarch %{multilib_64_archs}
`cat $(find %{gcc_target_platform}/libstdc++-v3/include -name c++config.h)`
%else
`cat $(find %{gcc_target_platform}/64/libstdc++-v3/include -name c++config.h)`
%endif
#endif
#endif
EOF
    break
  fi
done

for f in `find %{buildroot}%{_prefix}/include/c++/%{version}/%{gcc_target_platform}/ -name c++config.h`; do
  for i in 1 2 4 8; do
    sed -i -e 's/#define _GLIBCXX_ATOMIC_BUILTINS_'$i' 1/#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_'$i'\
&\
#endif/' $f
  done
done

# Nuke bits/stdc++.h.gch dirs
# 1) there is no bits/stdc++.h header installed, so when gch file can't be
#    used, compilation fails
# 2) sometimes it is hard to match the exact options used for building
#    libstdc++-v3 or they aren't desirable
# 3) there are multilib issues, conflicts etc. with this
# 4) it is huge
# People can always precompile on their own whatever they want, but
# shipping this for everybody is unnecessary.
rm -rf %{buildroot}%{_prefix}/include/c++/%{version}/%{gcc_target_platform}/bits/stdc++.h.gch

%ifarch sparcv9 sparc64
ln -f %{buildroot}%{_prefix}/bin/%{gcc_target_platform}-gcc \
  %{buildroot}%{_prefix}/bin/sparc-%{_vendor}-%{_target_os}-gcc
%endif
%ifarch ppc ppc64
ln -f %{buildroot}%{_prefix}/bin/%{gcc_target_platform}-gcc \
  %{buildroot}%{_prefix}/bin/ppc-%{_vendor}-%{_target_os}-gcc
%endif

find %{buildroot} -name \*.la | xargs rm -f

mv -f %{buildroot}%{_prefix}/%{_lib}/libgomp.spec $FULLPATH/

OBJDIR=`pwd`/%{gcc_target_platform}
pushd $FULLPATH
cp -a $OBJDIR/libstdc++-v3/src/.libs/libstdc++.so .
cp -a $OBJDIR/libgfortran/.libs/libgfortran.so .
cp -a $OBJDIR/libgcc/libgcc_s.so .
cp -a $OBJDIR/libgomp/.libs/libgomp.so .
echo 'INPUT ( %{_prefix}/%{_lib}/'`echo ../../../../%{_lib}/libmudflap.so.0.* | sed 's,^.*libm,libm,'`' )' > libmudflap.so
echo 'INPUT ( %{_prefix}/%{_lib}/'`echo ../../../../%{_lib}/libmudflapth.so.0.* | sed 's,^.*libm,libm,'`' )' > libmudflapth.so
mv -f %{buildroot}%{_prefix}/%{_lib}/libstdc++.*a $FULLPATH/
mv -f %{buildroot}%{_prefix}/%{_lib}/libsupc++.*a $FULLPATH/
mv -f %{buildroot}%{_prefix}/%{_lib}/libgfortran.*a $FULLPATH/
mv -f %{buildroot}%{_prefix}/%{_lib}/libgomp.*a $FULLPATH/
mv -f %{buildroot}%{_prefix}/%{_lib}/libmudflap{,th}.*a $FULLPATH/

%ifarch sparcv9 ppc
mkdir -p 64
cp -a $OBJDIR/64/libstdc++-v3/src/.libs/libstdc++.so 64/
cp -a $OBJDIR/64/libgfortran/.libs/libgfortran.so 64/
cp -a $OBJDIR/64/libgcc/64/libgcc_s.so 64/
cp -a $OBJDIR/64/libgomp/.libs/libgomp.so 64/
echo 'INPUT ( %{_prefix}/lib64/'`echo ../../../../lib/libmudflap.so.0.* | sed 's,^.*libm,libm,'`' )' > 64/libmudflap.so
echo 'INPUT ( %{_prefix}/lib64/'`echo ../../../../lib/libmudflapth.so.0.* | sed 's,^.*libm,libm,'`' )' > 64/libmudflapth.so
mv -f %{buildroot}%{_prefix}/lib64/libstdc++.*a 64/
mv -f %{buildroot}%{_prefix}/lib64/libsupc++.*a 64/
mv -f %{buildroot}%{_prefix}/lib64/libgfortran.*a 64/
mv -f %{buildroot}%{_prefix}/lib64/libgomp.*a 64/
mv -f %{buildroot}%{_prefix}/lib64/libmudflap{,th}.*a 64/
%endif
%ifarch %{multilib_64_archs}
mkdir -p 32
cp -a $OBJDIR/32/libstdc++-v3/src/.libs/libstdc++.so 32/
cp -a $OBJDIR/32/libgfortran/.libs/libgfortran.so 32/
cp -a $OBJDIR/32/libgcc/32/libgcc_s.so 32/
cp -a $OBJDIR/32/libgomp/.libs/libgomp.so 32/
echo 'INPUT ( %{_prefix}/lib/'`echo ../../../../lib64/libmudflap.so.0.* | sed 's,^.*libm,libm,'`' )' > 32/libmudflap.so
echo 'INPUT ( %{_prefix}/lib/'`echo ../../../../lib64/libmudflapth.so.0.* | sed 's,^.*libm,libm,'`' )' > 32/libmudflapth.so
mv -f %{buildroot}%{_prefix}/lib/libstdc++.*a 32/
mv -f %{buildroot}%{_prefix}/lib/libsupc++.*a 32/
mv -f %{buildroot}%{_prefix}/lib/libgfortran.*a 32/
mv -f %{buildroot}%{_prefix}/lib/libgomp.*a 32/
mv -f %{buildroot}%{_prefix}/lib/libmudflap{,th}.*a 32/
%endif

%ifarch ppc
mv -f $FULLPATH/libgcc_s.so $FULLPATH/libgcc_s_shared.so
echo '/* GNU ld script
   Use the shared library, but some functions are only in
   the static library, so try that secondarily.  */
OUTPUT_FORMAT(elf32-powerpc)
GROUP ( libgcc_s_shared.so libgcc.a )' > $FULLPATH/libgcc_s.so
%endif
%ifarch ppc64
mv -f $FULLPATH/32/libgcc_s.so $FULLPATH/32/libgcc_s_shared.so
echo '/* GNU ld script
   Use the shared library, but some functions are only in
   the static library, so try that secondarily.  */
OUTPUT_FORMAT(elf32-powerpc)
GROUP ( libgcc_s_shared.so libgcc.a )' > $FULLPATH/32/libgcc_s.so
%endif

# Strip debug info from Fortran/ObjC/Java static libraries
strip -g `find . \( -name libgfortran.a -o -name libgomp.a \
		    -o -name libmudflap.a -o -name libmudflapth.a \
		    -o -name libgcc.a -o -name libgcov.a \
		    -o -name libstdc++.a -o -name libsupc++.a \
		    -o -name libgcc_eh.a \) -a -type f`
chmod 755 %{buildroot}%{_prefix}/%{_lib}/libgfortran.so.3.*
chmod 755 %{buildroot}%{_prefix}/%{_lib}/libgomp.so.1.*
chmod 755 %{buildroot}%{_prefix}/%{_lib}/libmudflap{,th}.so.0.*

mv $FULLPATH/include-fixed/syslimits.h $FULLPATH/include/syslimits.h
mv $FULLPATH/include-fixed/limits.h $FULLPATH/include/limits.h
for h in `find $FULLPATH/include -name \*.h`; do
  if grep -q 'It has been auto-edited by fixincludes from' $h; then
    rh=`grep -A2 'It has been auto-edited by fixincludes from' $h | tail -1 | sed 's|^.*"\(.*\)".*$|\1|'`
    diff -up $rh $h || :
    rm -f $h
  fi
done

cd ..

for i in %{buildroot}%{_prefix}/bin/{*gcc,*++,gcov,gfortran}; do
  mv -f $i ${i}44
done
for i in %{buildroot}%{_mandir}/man1/{gcc,g++,gcov,gfortran}; do
  mv -f $i.1 ${i}44.1
done
for i in %{buildroot}%{_infodir}/{gcc,gfortran}; do
  mv -f $i.info.gz ${i}44.info.gz
done

# Remove binaries we will not be including, so that they don't end up in
# gcc-debuginfo
rm -f %{buildroot}%{_prefix}/%{_lib}/{libffi*,libiberty.a}
rm -f $FULLEPATH/install-tools/{mkheaders,fixincl}
rm -f %{buildroot}%{_prefix}/lib/{32,64}/libiberty.a
rm -f %{buildroot}%{_prefix}/%{_lib}/libssp*
rm -f %{buildroot}%{_prefix}/bin/%{gcc_target_platform}-{gcc*,c++*,g++*gfortran*}
rm -f %{buildroot}%{_prefix}/bin/cpp

rm -f %{buildroot}%{_prefix}/%{_lib}/lib*.so*
rm -f %{buildroot}%{_prefix}/%{_lib}/lib*.a
%ifarch %{multilib_64_archs}
# Remove libraries for the other arch on multilib arches
rm -f %{buildroot}%{_prefix}/lib/lib*.so*
rm -f %{buildroot}%{_prefix}/lib/lib*.a
%else
%ifarch sparcv9 ppc
rm -f %{buildroot}%{_prefix}/lib64/lib*.so*
rm -f %{buildroot}%{_prefix}/lib64/lib*.a
%endif
%endif

%check
cd obj-%{gcc_target_platform}

# run the tests.
make %{?_smp_mflags} -k check ALT_CC_UNDER_TEST=gcc ALT_CXX_UNDER_TEST=g++ RUNTESTFLAGS="--target_board=unix/'{,-fstack-protector}'" || :
echo ====================TESTING=========================
( LC_ALL=C ../contrib/test_summary || : ) 2>&1 | sed -n '/^cat.*EOF/,/^EOF/{/^cat.*EOF/d;/^EOF/d;/^LAST_UPDATED:/d;p;}'
echo ====================TESTING END=====================
mkdir testlogs-%{_target_platform}-%{version}-%{release}
for i in `find . -name \*.log | grep -F testsuite/ | grep -v 'config.log\|acats.*/tests/'`; do
  ln $i testlogs-%{_target_platform}-%{version}-%{release}/ || :
done
tar cf - testlogs-%{_target_platform}-%{version}-%{release} | bzip2 -9c \
  | uuencode testlogs-%{_target_platform}.tar.bz2 || :
rm -rf testlogs-%{_target_platform}-%{version}-%{release}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_prefix}/bin/gcc44
%{_prefix}/bin/gcov44
%{_mandir}/man1/gcc44.1*
%{_mandir}/man1/gcov44.1*
%{_infodir}/gcc44*
%dir %{_prefix}/lib/gcc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/cc1
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stddef.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stdarg.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stdfix.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/varargs.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/float.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/limits.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stdbool.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/iso646.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/syslimits.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/unwind.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/omp.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/mf-runtime.h
%ifarch %{ix86} x86_64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/mmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/xmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/emmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/pmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/tmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/ammintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/smmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/nmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/bmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/wmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/immintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/avxintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/x86intrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/fma4intrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/xopintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/lwpintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/abmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/popcntintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/bmiintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/tbmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/f16cintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/mm_malloc.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/mm3dnow.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/cpuid.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/cross-stdarg.h
%endif
%ifarch ia64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/ia64intrin.h
%endif
%ifarch ppc ppc64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/ppc-asm.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/altivec.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/spe.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/paired.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/ppu_intrinsics.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/si2vmx.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/spu2vmx.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/vec_types.h
%endif
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/collect2
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/crt*.o
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcov.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_eh.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_s.so
%ifarch ppc
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_s_shared.so
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgomp.spec
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgomp.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgomp.so
%ifarch sparcv9 ppc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/crt*.o
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcc.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcov.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcc_eh.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcc_s.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgomp.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgomp.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libmudflap.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libmudflapth.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libmudflap.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libmudflapth.so
%endif
%ifarch %{multilib_64_archs}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/crt*.o
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcc.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcov.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcc_eh.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcc_s.so
%ifarch ppc64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcc_s_shared.so
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgomp.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgomp.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libmudflap.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libmudflapth.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libmudflap.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libmudflapth.so
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libmudflap.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libmudflapth.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libmudflap.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libmudflapth.so
%doc gcc/README* rpm.doc/changelogs/gcc/ChangeLog* gcc/COPYING*
%doc rpm.doc/changelogs/libmudflap/ChangeLog*

%files c++
%defattr(-,root,root,-)
%{_prefix}/bin/g++44
%{_mandir}/man1/g++44.1*
%dir %{_prefix}/include/c++
%dir %{_prefix}/include/c++/%{version}
%{_prefix}/include/c++/%{version}/[^gjos]*
%{_prefix}/include/c++/%{version}/os*
%{_prefix}/include/c++/%{version}/s[^u]*
%dir %{_prefix}/lib/gcc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/cc1plus
%ifarch sparcv9 ppc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libstdc++.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libstdc++.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libsupc++.a
%endif
%ifarch %{multilib_64_archs}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libstdc++.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libstdc++.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libsupc++.a
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libstdc++.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libstdc++.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libsupc++.a
%doc rpm.doc/changelogs/gcc/cp/ChangeLog*
%doc rpm.doc/changelogs/libstdc++-v3/ChangeLog* libstdc++-v3/README*

%files gfortran
%defattr(-,root,root,-)
%{_prefix}/bin/gfortran44
%{_mandir}/man1/gfortran44.1*
%{_infodir}/gfortran44*
%dir %{_prefix}/lib/gcc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/finclude
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/finclude/omp_lib.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/finclude/omp_lib.f90
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/finclude/omp_lib.mod
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/finclude/omp_lib_kinds.mod
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/f951
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgfortranbegin.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgfortran.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgfortran.so
%ifarch sparcv9 ppc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgfortranbegin.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgfortran.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgfortran.so
%endif
%ifarch %{multilib_64_archs}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgfortranbegin.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgfortran.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgfortran.so
%endif
%doc rpm.doc/gfortran/*

%changelog
* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 4.4.7-8
- Mass rebuild 2014-01-24

* Tue Jan  7 2014 Jakub Jelinek <jakub@redhat.com> 4.4.7-7
- change -fstack-protector-strong to -fstack-protector in optflags,
  remove -march=x86-64 and -mfpmath=sse for ix86 (#1048852)

* Mon Jan  6 2014 Marek Polacek <polacek@redhat.com> 4.4.7-6
- #1043605: fix bogus CL date

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> 4.4.7-5
- mass rebuild 2013-12-27

* Fri Nov  1 2013 Marek Polacek <polacek@redhat.com> 4.4.7-4
- don't obsolete compat-libstdc++-33 (#1031748)

* Sat Jul 20 2013 Jakub Jelinek <jakub@redhat.com> 4.4.7-3
- new compat package
