%define DATE 20060404
%define _unpackaged_files_terminate_build 0
%define multilib_64_archs sparc64 ppc64 s390x x86_64
%ifarch s390x
%define multilib_32_arch s390
%endif
%ifarch sparc64
%define multilib_32_arch sparc
%endif
%ifarch ppc64
%define multilib_32_arch ppc
%endif
%ifarch x86_64
%define multilib_32_arch i386
%endif
Summary: Compatibility GNU Compiler Collection
Name: compat-gcc-34
Version: 3.4.6
Release: 41%{?dist}
# libgcc and crtstuff have an exception which allows
# linking it into any kind of programs or shared libraries without
# restrictions.
License: GPLv2+ and GPLv2+ with exceptions
Group: Development/Languages
Source0: gcc-%{version}-%{DATE}.tar.bz2
Source1: dummylib.sh
URL: http://gcc.gnu.org
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
# Need .eh_frame ld optimizations
# Need proper visibility support
# Need -pie support
# Need --as-needed/--no-as-needed support
# Need .weakref support
BuildRequires: binutils >= 2.16.91.0.5-1
BuildRequires: zlib-devel, gettext, dejagnu, bison, flex, texinfo
# Make sure pthread.h doesn't contain __thread tokens
BuildRequires: glibc-devel >= 2.2.90-12
BuildRequires: %{_prefix}/share/i18n/locales/de_DE
# Need .eh_frame ld optimizations
# Need proper visibility support
# Need -pie support
# Need .weakref support
Requires: binutils >= 2.16.91.0.5-1
# Make sure gdb will understand DW_FORM_strp
Conflicts: gdb < 5.1-2
Requires: glibc-devel >= 2.2.90-12
Requires: libgcc >= 4.1.0
BuildRequires: elfutils-devel >= 0.72
ExcludeArch: %{arm} aarch64 ppc64le
%ifarch %{multilib_64_archs} sparc sparcv9 ppc
# Ensure glibc{,-devel} is installed for both multilib arches
BuildRequires: /lib/libc.so.6 /usr/lib/libc.so /lib64/libc.so.6 /usr/lib64/libc.so
%endif
Obsoletes: compat-egcs
Obsoletes: compat-gcc
Obsoletes: compat-gcc-objc
Obsoletes: compat-egcs-objc
Obsoletes: compat-gcc-g77
Obsoletes: compat-egcs-g77
Obsoletes: compat-gcc-java
Obsoletes: compat-libgcj
Obsoletes: compat-libgcj-devel
Provides: bundled(libiberty)

Patch1: gcc34-multi32-hack.patch
Patch2: gcc34-ice-hack.patch
Patch3: gcc34-ppc64-m32-m64-multilib-only.patch
Patch4: gcc34-ia64-lib64.patch
Patch5: gcc34-java-nomulti.patch
Patch6: gcc34-gnuc-rh-release.patch
Patch7: gcc34-pr16104.patch
Patch8: gcc34-var-tracking-fix.patch
Patch9: gcc34-i386-movsi-insv.patch
Patch10: gcc34-pr18925.patch
Patch11: gcc34-pr14084.patch
Patch12: gcc34-hashtab-recursion.patch
Patch13: gcc34-java-jnilink.patch
Patch14: gcc34-pr21955.patch
Patch15: gcc34-vsb-stack.patch
Patch16: gcc34-pr18300.patch
Patch17: gcc34-rh156291.patch
Patch18: gcc34-weakref.patch
Patch19: gcc34-dwarf2-usefbreg.patch
Patch20: gcc34-dwarf2-prefer-1elt-vartracking.patch
Patch21: gcc34-dwarf2-pr20268.patch
Patch22: gcc34-dwarf2-inline-details.patch
Patch23: gcc34-dwarf2-frame_base.patch
Patch24: gcc34-dwarf2-i386-multreg1.patch
Patch25: gcc34-dwarf2-i386-multreg2.patch
Patch26: gcc34-rh176182.patch
Patch27: gcc34-pr11953.patch
Patch28: gcc34-pr23591.patch
Patch29: gcc34-pr26208.patch
Patch30: gcc34-pr8788.patch
Patch31: gcc34-rh137200.patch
Patch32: gcc34-rh172117.patch
Patch33: gcc34-rh172876.patch
Patch34: gcc34-rh178062.patch
Patch35: gcc34-pr21412.patch
Patch36: gcc34-sw2438.patch
Patch37: gcc34-pr26208-workaround.patch
Patch38: gcc34-libgcc_eh-hidden.patch
Patch39: gcc34-frame-base-loclist.patch
Patch40: gcc34-CVE-2006-3619.patch
Patch41: gcc34-dwarf2-inline-details-fix.patch
Patch42: gcc34-CXXABI131.patch
Patch43: gcc34-rh205919.patch
Patch44: gcc34-rh207277.patch
Patch45: gcc34-var-tracking-coalesce.patch
Patch46: gcc34-java-zoneinfo.patch
Patch47: gcc34-libgcc-additions.patch
Patch48: gcc34-pr24975.patch
Patch49: gcc34-rh233941.patch
Patch50: gcc34-rh234515.patch
Patch51: gcc34-rh235008.patch
Patch52: gcc34-rh235255.patch
Patch53: gcc34-rh242685.patch

Patch100: gcc34-ldbl-hack.patch
Patch101: gcc34-makeinfo.patch
Patch102: gcc34-bison4.patch
Patch103: gcc34-pr56258.patch
Patch104: gcc34-bison.patch

%define _gnu %{nil}
%ifarch sparc sparcv9
%define gcc_target_platform sparc64-%{_vendor}-linux
%endif
%ifarch ppc
%define gcc_target_platform ppc64-%{_vendor}-linux
%endif
%ifnarch sparc sparcv9 ppc
%define gcc_target_platform %{_target_cpu}-%{_vendor}-linux
%endif

%description
This package includes a GCC 3.4.6-RH compatibility compiler.

%package c++
Summary: C++ support for compatibility compiler
Group: Development/Languages
Requires: compat-gcc-34 = %{version}-%{release}
Requires: libstdc++ >= 4.1.0, libstdc++ < 7.0.0
Obsoletes: compat-egcs-c++
Obsoletes: compat-gcc-c++
Obsoletes: compat-libstdc++
Obsoletes: compat-libstdc++-devel
AutoProv: false

%description c++
This package includes a GCC 3.4.6-RH compatibility C++ compiler.

%package g77
Summary: Fortran 77 support for compatibility compiler
Group: Development/Languages
Requires: compat-gcc-34 = %{version}-%{release}
Requires: compat-libf2c-34 = %{version}-%{release}
Requires(post): /sbin/install-info
Requires(preun): /sbin/install-info
Obsoletes: gcc3-g77
Obsoletes: gcc-g77
Autoreq: true

%description g77
The compat-gcc-34-g77 package provides support for compiling Fortran 77
programs with the GNU Compiler Collection.

%package -n compat-libf2c-34
Summary: Fortran 77 compatibility runtime
Group: System Environment/Libraries
Autoreq: true
Obsoletes: libf2c, compat-libf2c-32

%description -n compat-libf2c-34
This package contains Fortran 77 shared library which is needed to run
Fortran 77 dynamically linked programs.

%prep
%setup -q -n gcc-%{version}-%{DATE}
%ifarch sparc sparcv9 ppc
%patch1 -p0 -b .multi32-hack~
%endif
%patch2 -p0 -b .ice-hack~
%patch3 -p0 -b .ppc64-m32-m64-multilib-only~
%ifarch ia64
%if "%{_lib}" == "lib64"
%patch4 -p0 -b .ia64-lib64~
%endif
%endif
%patch5 -p0 -b .java-nomulti~
%patch6 -p0 -b .gnuc-rh-release~
%patch7 -p0 -b .pr16104~
%patch8 -p0 -b .var-tracking-fix~
%patch9 -p0 -b .i386-movsi-insv~
%patch10 -p0 -b .pr18925~
%patch11 -p0 -b .pr14084~
%patch12 -p0 -b .hashtab-recursion~
%patch13 -p0 -b .java-jnilink~
%patch14 -p0 -b .pr21955~
%patch15 -p0 -b .vsb-stack~
%patch16 -p0 -b .pr18300~
%patch17 -p0 -b .rh156291~
%patch18 -p0 -b .weakref~
%patch19 -p0 -b .dwarf2-usefbreg~
%patch20 -p0 -b .dwarf2-prefer-1elt-vartracking~
%patch21 -p0 -b .dwarf2-pr20268~
%patch22 -p0 -b .dwarf2-inline-details~
%patch23 -p0 -b .dwarf2-frame_base~
%patch24 -p0 -b .dwarf2-i386-multreg1~
%patch25 -p0 -b .dwarf2-i386-multreg2~
%patch26 -p0 -b .rh176182~
%patch27 -p0 -b .pr11953~
%patch28 -p0 -b .pr23591~
%patch29 -p0 -b .pr26208~
%patch30 -p0 -b .pr8788~
%patch31 -p0 -b .rh137200~
%patch32 -p0 -b .rh172117~
%patch33 -p0 -b .rh172876~
%patch34 -p0 -b .rh178062~
%patch35 -p0 -b .pr21412~
%patch36 -p0 -b .sw2438~
%patch37 -p0 -b .pr26208-workaround~
%patch38 -p0 -b .libgcc_eh-hidden~
%patch39 -p0 -b .frame-base-loclist~
%patch40 -p0 -b .CVE-2006-3619~
%patch41 -p0 -b .dwarf2-inline-details-fix~
%patch42 -p0 -b .CXXABI131~
%patch43 -p0 -b .rh205919~
%patch44 -p0 -b .rh207277~
%patch45 -p0 -b .var-tracking-coalesce~
%patch46 -p0 -b .java-zoneinfo~
%patch47 -p0 -b .libgcc-additions~
%patch48 -p0 -b .pr24975~
%patch49 -p0 -b .rh233941~
%patch50 -p0 -b .rh234515~
%patch51 -p0 -b .rh235008~
%patch52 -p0 -b .rh235255~
%patch53 -p0 -b .rh242685~

%patch100 -p0 -b .ldbl-hack~
%patch101 -p0 -b .makeinfo~
%patch102 -p0 -b .bison4~
%patch103 -p0 -b .pr56258~
%patch104 -p0 -b .bison~

sed -i -e 's/struct siginfo/siginfo_t/' gcc/config/*/linux*.h

perl -pi -e 's/3\.4\.7/3.4.6/' gcc/version.c
perl -pi -e 's/"%{version}"/"%{version} \(release\)"/' gcc/version.c
perl -pi -e 's/\((prerelease|experimental|release|Red Hat[^)]*)\)/\(Red Hat %{version}-%{release}\)/' gcc/version.c

# Misdesign in libstdc++
cp -a libstdc++-v3/config/cpu/i{4,3}86/atomicity.h

./contrib/gcc_update --touch

%build

rm -fr obj-%{gcc_target_platform}
mkdir obj-%{gcc_target_platform}
cd obj-%{gcc_target_platform}

mkdir -p ld_hack
cat > ld_hack/ld <<\EOF
#!/bin/sh
case " $* " in *\ -r\ *) exec /usr/bin/ld "$@";; esac
exec /usr/bin/ld --build-id "$@"
EOF
chmod 755 ld_hack/ld
export PATH=`pwd`/ld_hack/${PATH:+:$PATH}

if [ ! -f /usr/lib/locale/de_DE/LC_CTYPE ]; then
  mkdir locale
  localedef -f ISO-8859-1 -i de_DE locale/de_DE
  export LOCPATH=`pwd`/locale:/usr/lib/locale
fi

CC='gcc -std=gnu89'
OPT_FLAGS=`echo $RPM_OPT_FLAGS|sed -e 's/-fno-rtti//g' -e 's/-fno-exceptions//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-m64//g;s/-m32//g;s/-m31//g'`
%ifarch %{ix86}
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mtune=pentium4/-mtune=i686/g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mtune=generic/-mtune=i686/g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mtune=atom/-mtune=i686/g'`
%endif
%ifarch x86_64
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mtune=nocona//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mtune=generic//g'`
%endif
%ifarch sparc sparcv9 sparc64
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-mcpu=ultrasparc/-mtune=ultrasparc/g'`
%endif
%ifarch s390 s390x
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-march=z9-109//g;s/-mtune=z10//g'`
%endif
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-Wall//g' -e 's/-Wp,-D_FORTIFY_SOURCE=2//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-fexceptions//g' -e 's/-fasynchronous-unwind-tables//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-fstack-protector\(-strong\)\?//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/--param=ssp-buffer-size=[0-9]*//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-grecord-gcc-switches//g'`
OPT_FLAGS=`echo $OPT_FLAGS|sed -e 's/-Werror=format-security//g'`
%ifarch sparc64
cat > gcc64 <<"EOF"
#!/bin/sh
exec /usr/bin/gcc -std=gnu89 -m64 "$@"
EOF
chmod +x gcc64
CC=`pwd`/gcc64
%endif
%ifarch ppc64
if gcc -m64 -xc -S /dev/null -o - > /dev/null 2>&1; then
  cat > gcc64 <<"EOF"
#!/bin/sh
exec /usr/bin/gcc -std=gnu89 -m64 "$@"
EOF
  chmod +x gcc64
  CC=`pwd`/gcc64
fi
%endif
CC="$CC" CFLAGS="$OPT_FLAGS" CXXFLAGS="$OPT_FLAGS" XCFLAGS="$OPT_FLAGS" TCFLAGS="$OPT_FLAGS" \
	GCJFLAGS="$OPT_FLAGS" \
	../configure --prefix=%{_prefix} --mandir=%{_mandir} --infodir=%{_infodir} \
	--enable-shared --enable-threads=posix --disable-checking \
	--with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions \
	--enable-languages=c,c++,f77 --disable-libgcj \
%ifarch sparc sparcv9
	--host=%{gcc_target_platform} --build=%{gcc_target_platform} --target=%{gcc_target_platform} --with-cpu=v7
%endif
%ifarch ppc
	--host=%{gcc_target_platform} --build=%{gcc_target_platform} --target=%{gcc_target_platform} --with-cpu=default32
%endif
%ifnarch sparc sparcv9 ppc
	--host=%{gcc_target_platform}
%endif

%ifarch %{ix86} x86_64
make %{?_smp_mflags} BOOT_CFLAGS="$OPT_FLAGS" profiledbootstrap
%else
make %{?_smp_mflags} BOOT_CFLAGS="$OPT_FLAGS" bootstrap-lean
%endif

# Fix up libstdc++.so's
d_first=yes
for d in `pwd`/%{gcc_target_platform}/libstdc++-v3 `pwd`/%{gcc_target_platform}/*/libstdc++-v3; do
  test -d $d || continue
  pushd $d/src
    sh %{SOURCE1} .libs/libstdc++.so .libs/ll.so libstdc++-symbol.ver
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

# Make sure we are using system libgcc_s, as system libstdc++.so.6 might
# use unwinding features that require it.
mv gcc/libgcc_s.so.1{,.bak}
ln -sf /%{_lib}/libgcc_s.so.1 gcc/libgcc_s.so.1

# run the tests.
make %{?_smp_mflags} -k check || :
echo ====================TESTING=========================
( ../contrib/test_summary || : ) 2>&1 | sed -n '/^cat.*EOF/,/^EOF/{/^cat.*EOF/d;/^EOF/d;/^LAST_UPDATED:/d;p;}'
echo ====================TESTING END=====================

%install
rm -fr $RPM_BUILD_ROOT

export PATH=`pwd`/obj-%{gcc_target_platform}/ld_hack/${PATH:+:$PATH}

perl -pi -e \
  's~href="l(ibstdc|atest)~href="http://gcc.gnu.org/onlinedocs/libstdc++/l\1~' \
  libstdc++-v3/docs/html/documentation.html
ln -sf documentation.html libstdc++-v3/docs/html/index.html
find libstdc++-v3/docs/html -name CVS | xargs rm -rf

cd obj-%{gcc_target_platform}

if [ ! -f /usr/lib/locale/de_DE/LC_CTYPE ]; then
  export LOCPATH=`pwd`/locale:/usr/lib/locale
fi

TARGET_PLATFORM=%{gcc_target_platform}

make prefix=$RPM_BUILD_ROOT%{_prefix} mandir=$RPM_BUILD_ROOT%{_mandir} \
  infodir=$RPM_BUILD_ROOT%{_infodir} install

FULLPATH=$RPM_BUILD_ROOT%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
FULLEPATH=$RPM_BUILD_ROOT%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}

cxxconfig="`find %{gcc_target_platform}/libstdc++-v3/include -name c++config.h`"
for i in `find %{gcc_target_platform}/[36]*/libstdc++-v3/include -name c++config.h 2>/dev/null`; do
  if ! diff -up $cxxconfig $i; then
    cat > $RPM_BUILD_ROOT%{_prefix}/include/c++/%{version}/%{gcc_target_platform}/bits/c++config.h <<EOF
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

mkdir -p $RPM_BUILD_ROOT/%{_lib}
mv -f $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libgcc_s.so.1 $RPM_BUILD_ROOT/%{_lib}/libgcc_s-%{version}-%{DATE}.so.1
chmod 755 $RPM_BUILD_ROOT/%{_lib}/libgcc_s-%{version}-%{DATE}.so.1
ln -sf libgcc_s-%{version}-%{DATE}.so.1 $RPM_BUILD_ROOT/%{_lib}/libgcc_s.so.1
ln -sf /%{_lib}/libgcc_s.so.1 $FULLPATH/libgcc_s.so
%ifarch sparc sparcv9 ppc
ln -sf /lib64/libgcc_s.so.1 $FULLPATH/libgcc_s_64.so
%endif
%ifarch %{multilib_64_archs}
ln -sf /lib/libgcc_s.so.1 $FULLPATH/libgcc_s_32.so
%endif

for h in `find $FULLPATH/include -name \*.h`; do
  if grep -q 'It has been auto-edited by fixincludes from' $h; then
    rh=`grep -A2 'It has been auto-edited by fixincludes from' $h | tail -1 | sed 's|^.*"\(.*\)".*$|\1|'`
    diff -up $rh $h || :
    rm -f $h
  fi
done

cd ..

%ifarch ppc ppc64 s390 s390x
# GCC 3.4.x always uses IEEE double long double type on ppc*/s390*
# Although properly written programs should use headers which will DTRT,
# the addition of -lnldbl_nonshared should fix even buggy programs
# and shared libraries.
sed -i -e 's/%G %L/%G -lnldbl_nonshared %L/' \
  $RPM_BUILD_ROOT%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/specs
%endif

%ifarch sparc sparcv9 ppc
ln -f $RPM_BUILD_ROOT%{_prefix}/bin/%{gcc_target_platform}-gcc \
  $RPM_BUILD_ROOT%{_prefix}/bin/%{_target_platform}-gcc
%endif
%ifarch sparc64
ln -f $RPM_BUILD_ROOT%{_prefix}/bin/%{gcc_target_platform}-gcc \
  $RPM_BUILD_ROOT%{_prefix}/bin/sparc-%{_vendor}-%{_target_os}-gcc
%endif
%ifarch ppc64
ln -f $RPM_BUILD_ROOT%{_prefix}/bin/%{gcc_target_platform}-gcc \
  $RPM_BUILD_ROOT%{_prefix}/bin/ppc-%{_vendor}-%{_target_os}-gcc
%endif

for i in $RPM_BUILD_ROOT%{_prefix}/bin/{*gcc,*++,gcov}; do
  mv -f $i ${i}34
done

ln -sf g77 $RPM_BUILD_ROOT%{_prefix}/bin/f77
target_libdir=`pwd`/obj-%{gcc_target_platform}/%{gcc_target_platform}/
pushd $FULLPATH
mv -f $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libg2c.*a .
mv -f $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libfrtbegin.*a .
mv -f $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libstdc++.*a .
mv -f $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libsupc++.*a .
mv -f $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libstdc++_nonshared.*a .
cp -a $target_libdir/libstdc++-v3/src/.libs/ll.so libstdc++_shared.so
echo 'GROUP ( -lstdc++_nonshared -lstdc++_shared )' > libstdc++.so
if [ "%{_lib}" = "lib" ]; then
ln -sf ../../../libg2c.so.0.* libg2c.so
else
ln -sf ../../../../%{_lib}/libg2c.so.0.* libg2c.so
fi
%ifarch sparc sparcv9 ppc
mkdir -p 64
ln -sf ../`echo ../../../../lib/libg2c.so.0.* | sed s~/lib/~/lib64/~` 64/libg2c.so
mv -f $RPM_BUILD_ROOT%{_prefix}/lib64/libg2c.*a 64/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib64/libfrtbegin.*a 64/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib64/libstdc++.*a 64/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib64/libsupc++.*a 64/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib64/libstdc++_nonshared.*a 64/
cp -a $target_libdir/64/libstdc++-v3/src/.libs/ll.so 64/libstdc++_shared.so
echo 'GROUP ( -lstdc++_nonshared -lstdc++_shared )' > 64/libstdc++.so
%endif
%ifarch %{multilib_64_archs}
mkdir -p 32
ln -sf ../`echo ../../../../lib64/libg2c.so.0.* | sed s~/../lib64/~/~` 32/libg2c.so
mv -f $RPM_BUILD_ROOT%{_prefix}/lib/libg2c.*a 32/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib/libfrtbegin.*a 32/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib/libstdc++.*a 32/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib/libsupc++.*a 32/
mv -f $RPM_BUILD_ROOT%{_prefix}/lib/libstdc++_nonshared.*a 32/
cp -a $target_libdir/32/libstdc++-v3/src/.libs/ll.so 32/libstdc++_shared.so
echo 'GROUP ( -lstdc++_nonshared -lstdc++_shared )' > 32/libstdc++.so
%endif
popd

# Strip debug info from Fortran static libraries
strip -g `find . \( -name libg2c.a -o -name libfrtbegin.a \
		    -o -name libstdc++\*.a -o -name libsupc++.a \) -a -type f`
chmod 755 $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libg2c.so.0.*

rm -f $RPM_BUILD_ROOT%{_prefix}/lib*/libiberty.a
rm -f $RPM_BUILD_ROOT/lib*/libgcc_s*
rm -f $RPM_BUILD_ROOT%{_prefix}/bin/cpp

%ifarch %{multilib_64_archs}
# Remove libraries for the other arch on multilib arches
rm -f $RPM_BUILD_ROOT%{_prefix}/lib/lib*.so*
rm -f $RPM_BUILD_ROOT%{_prefix}/lib/lib*.a
%else
%ifarch sparc sparcv9 ppc
rm -f $RPM_BUILD_ROOT%{_prefix}/lib64/lib*.so*
rm -f $RPM_BUILD_ROOT%{_prefix}/lib64/lib*.a
%endif
%endif

# libstdc++.so.6.0.x is included in libstdc++ package, not here.
rm -rf $RPM_BUILD_ROOT%{_prefix}/%{_lib}/libstdc++.so*

%clean
rm -rf $RPM_BUILD_ROOT

%post g77
/sbin/install-info \
  --info-dir=%{_infodir} %{_infodir}/g77.info.gz || :

%preun g77
if [ $1 = 0 ]; then
  /sbin/install-info --delete \
    --info-dir=%{_infodir} %{_infodir}/g77.info.gz || :
fi

%post -n compat-libf2c-34 -p /sbin/ldconfig

%postun -n compat-libf2c-34 -p /sbin/ldconfig

%files
%defattr(-,root,root)
%{_prefix}/bin/gcc34
%{_prefix}/bin/gcov34
%ifarch sparc sparcv9 ppc
%{_prefix}/bin/%{_target_platform}-gcc34
%endif
%ifarch sparc64
%{_prefix}/bin/sparc-%{_vendor}-%{_target_os}-gcc34
%endif
%ifarch ppc64
%{_prefix}/bin/ppc-%{_vendor}-%{_target_os}-gcc34
%endif
%{_prefix}/bin/%{gcc_target_platform}-gcc34
%dir %{_prefix}/lib/gcc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stddef.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stdarg.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/varargs.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/float.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/limits.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/stdbool.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/iso646.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/syslimits.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/unwind.h
%ifarch %{ix86} x86_64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/mmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/xmmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/emmintrin.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/pmmintrin.h
%endif
%ifarch ia64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/ia64intrin.h
%endif
%ifarch ppc ppc64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/ppc-asm.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/altivec.h
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/spe.h
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/README
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/collect2
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/cc1
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/crt*.o
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcov.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_eh.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_s.so
%ifarch sparc sparcv9 ppc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/crt*.o
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcc.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcov.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libgcc_eh.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_s_64.so
%endif
%ifarch %{multilib_64_archs}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/crt*.o
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcc.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcov.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libgcc_eh.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libgcc_s_32.so
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/specs
%doc gcc/README* gcc/*ChangeLog* gcc/COPYING*

%files c++
%defattr(-,root,root)
%{_prefix}/bin/%{gcc_target_platform}-*++34
%{_prefix}/bin/g++34
%dir %{_prefix}/lib/gcc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/cc1plus
%ifarch sparc sparcv9 ppc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libstdc++.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libstdc++_shared.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libstdc++.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libstdc++_nonshared.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libsupc++.a
%endif
%ifarch %{multilib_64_archs}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libstdc++.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libstdc++_shared.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libstdc++.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libstdc++_nonshared.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libsupc++.a
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libstdc++.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libstdc++_shared.so
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libstdc++.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libstdc++_nonshared.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libsupc++.a
%{_prefix}/include/c++
%doc gcc/cp/ChangeLog*
%doc libstdc++-v3/ChangeLog* libstdc++-v3/README* libstdc++-v3/docs/html/

%files g77
%defattr(-,root,root)
%{_prefix}/bin/g77
%{_prefix}/bin/f77
%{_mandir}/man1/g77.1*
%{_infodir}/g77*
%dir %{_prefix}/lib/gcc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}
%dir %{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}
%{_prefix}/libexec/gcc/%{gcc_target_platform}/%{version}/f771
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libfrtbegin.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libg2c.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/libg2c.so
%ifarch sparc sparcv9 ppc
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libfrtbegin.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libg2c.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/64/libg2c.so
%endif
%ifarch %{multilib_64_archs}
%dir %{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libfrtbegin.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libg2c.a
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/32/libg2c.so
%endif
%{_prefix}/lib/gcc/%{gcc_target_platform}/%{version}/include/g2c.h
%doc gcc/f/ChangeLog*

%files -n compat-libf2c-34
%defattr(-,root,root)
%{_prefix}/%{_lib}/libg2c.so.0*

%changelog
* Fri Aug 12 2016 Peter Robinson <pbrobinson@fedoraproject.org> 3.4.6-41
- excludearch aarch64, unsupported in older gcc

* Thu May  5 2016 Jakub Jelinek  <jakub@redhat.com> 3.4.6-40
- buildrequire data files for localedef
- use -std=gnu89 in the host compiler

* Fri Mar 18 2016 Jakub Jelinek  <jakub@redhat.com> 3.4.6-39
- don't conflict with libstdc++ 6.x, it is still backwards ABI compatible

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 3.4.6-38
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-37
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue Feb 24 2015 Jakub Jelinek  <jakub@redhat.com> 3.4.6-36
- don't conflict with libstdc++ 5.x, it is still backwards ABI compatible

* Tue Oct 14 2014 Karsten Hopp <karsten@redhat.com> 3.4.6-35
- excludearch ppc64le, unsupported in older gcc

* Mon Sep 22 2014 Jakub Jelinek  <jakub@redhat.com> 3.4.6-34
- don't conflict with libstdc++ 4.9.x, it is still backwards ABI compatible
- fix build on F21+

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-33
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-32
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Mon Aug 19 2013 Peter Robinson <pbrobinson@fedoraproject.org> 3.4.6-31
- Exclude ARM as HFP was not supported until gcc 4.5

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-30
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Wed Feb 20 2013 Jakub Jelinek  <jakub@redhat.com> 3.4.6-29
- don't conflict with libstdc++ 4.8.x, it is still backwards ABI compatible

* Wed Feb 20 2013 Jakub Jelinek  <jakub@redhat.com> 3.4.6-28
- use siginfo_t instead of struct siginfo
- fix build with makeinfo >= 5.0 (PR bootstrap/56258)

* Wed Feb 13 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-27
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Mon Oct 15 2012 Jon Ciesla <limburgher@gmail.com> - 3.4.6-26
- Provides: bundled(libiberty)

* Wed Jul 18 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-25
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Feb  1 2012 Jakub Jelinek  <jakub@redhat.com> 3.4.6-24
- don't conflict with libstdc++ 4.7.x, it is still backwards ABI compatible

* Thu Jan 12 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-23
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-22
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Fri Jan 28 2011 Jakub Jelinek  <jakub@redhat.com> 3.4.6-21
- don't conflict with libstdc++ 4.6.x, it is still backwards ABI compatible

* Thu Jul  8 2010 Jakub Jelinek  <jakub@redhat.com> 3.4.6-20
- don't conflict with libstdc++ 4.5.x, it is still backwards ABI compatible

* Mon Jun 28 2010 Jakub Jelinek  <jakub@redhat.com> 3.4.6-19
- add %%{?dist} (#604538)

* Mon Nov 30 2009 Dennis Gregorovic <dgregor@redhat.com> - 3.4.6-18.1
- Rebuilt for RHEL 6

* Mon Aug 31 2009 Karsten Hopp <karsten@redhat.com> 3.4.6-18
- strip -march=z9-109 and -mtune=z10 from OPT_FLAGS on s390, s390x
  (#519507)

* Fri Jul 31 2009 Jakub Jelinek  <jakub@redhat.com> 3.4.6-17
- make sure to use system libgcc_s.so.1 instead of gcc34 one during
  testing

* Tue Jul 28 2009 Jakub Jelinek  <jakub@redhat.com> 3.4.6-16
- replace -mtune=atom in $RPM_OPT_FLAGS with something that
  GCC 3.4.6 groks

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> 3.4.6-15
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Fri Jun 12 2009 Dennis Gilmore <dennis@ausil.us> - 3.4.6-14
- setup to build sparcv9

* Tue Feb 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.4.6-13
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Tue Feb 10 2009 Jakub Jelinek  <jakub@redhat.com> 3.4.6-12
- rebuild with gcc 4.4, allow libstdc++ 4.1.0 ... 4.4.x

* Fri Feb  1 2008 Jakub Jelinek  <jakub@redhat.com> 3.4.6-9
- rebuild with gcc 4.3, allow libstdc++ 4.1.0 ... 4.3.x

* Tue Oct 16 2007 Jakub Jelinek  <jakub@redhat.com> 3.4.6-8
- update License tag
- build with ld --build-id
- grok makeinfo >= 4.10 during configury
- avoid aliasing warnings in libstdc++-v3 headers when compiled
  with g++ 4.x (Paolo Carlini, PR libstdc++/24975, #240020)
- fix RTL expansion of COMPLEX_EXPR (#233941)
- fix deque<>::erase(iterator, iterator) (Steve LoBasso,
  Paolo Carlini, #234515)
- fix french and kinyarwanda translations (#235008)
- handle PARALLELs in GCSE store motion (Alexandre Oliva, #235255)
- ensure zero termination for invalid, overly long, std::__enc_traits
  internal or external character set names (Jatin Nansi, #242685)

* Sat Mar  3 2007 Jakub Jelinek  <jakub@redhat.com> 3.4.6-7
- ignore install-info failures in scriptlets (#223680)
- don't include cpp.debug in compat-gcc-34-debuginfo (#227021)
- fix .debug_line for inline function parameter blocks
  (Alexandre Oliva, #214353)
- fix hang in vt_find_locations with -O{2,3} -g
  (Alexandre Oliva, #216695, #218377)
- fix if-conversion ICE (Eric Botcazou, #207277)
- fix template instantiation ICE (Alexandre Oliva, #205919)

* Wed Aug 23 2006 Jakub Jelinek  <jakub@redhat.com> 3.4.6-4
- buildrequire elfutils-devel, so that libgcc_s is properly built
  on ia64
- on ppc*/s390* make sure all needed math *l stubs are included
- add -lnldbl_nonshared to ppc*/s390* specs

* Wed Aug  9 2006 Jakub Jelinek  <jakub@redhat.com> 3.4.6-3
- new compat package
