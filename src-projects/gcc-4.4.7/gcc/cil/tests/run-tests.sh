#!/bin/bash

if [ $# -eq 0 ] ; then
    TESTS="args.c  array.c  attribute.c  bitfield.c bitfield-r1.c bitfield-r2.c bitfield-r3.c bitfieldxc.c bsort.c cvolatile.c deref_struct.c div.c empty.c  enum.c  enum_op.c float_call.c getbitsx.c icall.c initialization.c longlong.c lscope_struct.c pinvoke.c printf.c  qsort.c qsort-r.c radial33.c s0.c  s1.c  s2.c  s3.c  scalar.c string.c struct_op.c  struct_op-r.c struct_op-t.c struct_union.c switch.c switch2.c switch-r.c vec.c"
else
    TESTS="$@"
fi
TESTSDIR="$(dirname $0)"


CIL32GCC="/opt/gcc4NET/bin/cil32-gcc"
GCCCIL_PREFIX="/home/ricardo/mine/install"
GCCCIL="${GCCCIL_PREFIX}/bin/gcil"
#GCCCIL_OPTIONS=" -fuse-mono-runtime"
GCCCIL_OPTIONS=""
VM='mono'
DIS='monodis'

EXTRACIL1O3="-mtune=generic"

function e ()
{
    LOG="$1"
    shift
    echo $@ >> $LOG
    $@ >> $LOG 2>&1
} 

function eo ()
{
    LOG="$1"
    shift
    OUT="$1"
    shift
    echo $@ \> $OUT >> $LOG
    $@ > $OUT 2>> $LOG
} 

function eoe ()
{
    LOG="$1"
    shift
    OUT="$1"
    shift
    echo $@ \&\> $OUT >> $LOG
    $@ &> $OUT
} 

for i in $TESTS ; do
    f=${TESTSDIR}/$i
    ib="$(echo $(basename $f) | rev | cut -d. -f2- | rev)"
    echo -n "${ib} "
    echo "$ib"  > "$ib".log

    e "$ib".log ${CIL32GCC} "$f" -S -o "${ib}.il"
    if [ $? -ne 0 ] ; then echo "FAILED compiling C to IL" ; continue ; fi

    e "$ib".log ${CIL32GCC} "$f" -o "${ib}.exe"
    if [ $? -ne 0 ] ; then echo "FAILED compiling C to EXE" ; continue ; fi

    eo "$ib".log ${ib}.exe.dis ${DIS} ${ib}.exe 
    if [ $? -ne 0 ] ; then echo "FAILED disassembling EXE" ; continue ; fi

    e "$ib".log ${GCCCIL} -S "${ib}.exe" ${GCCCIL_OPTIONS} -o "${ib}.exe.s"
    if [ $? -ne 0 ] ; then echo "FAILED compiling EXE to assembler" ; continue ; fi


    e "$ib".log ${GCCCIL} -v "${ib}.exe" ${GCCCIL_OPTIONS} -lm -o "${ib}"
    if [ $? -ne 0 ] ; then echo "FAILED compiling EXE to executable" ; continue ; fi

    eoe "$ib".log "${ib}.vmout" ${VM} "${ib}.exe"
    VM_EXITCODE=$?
    echo "Vm output: <<EOF"  >> "$ib".log 2>&1
    cat "${ib}.vmout"  >> "$ib".log 2>&1
    echo "EOF"  >> "$ib".log 2>&1
    echo "Vm exitcode: ${VM_EXITCODE}" >> "$ib".log 2>&1

    eoe "$ib".log "${ib}.out" "./${ib}"
    EXECUTABLE_EXITCODE=$?
    echo "Executable output: <<EOF"  >> "$ib".log 2>&1
    cat "${ib}.out"  >> "$ib".log 2>&1
    echo "EOF"  >> "$ib".log 2>&1
    echo "Exitcode: ${EXECUTABLE_EXITCODE}" >> "$ib".log 2>&1

    if [ $VM_EXITCODE -eq $EXECUTABLE_EXITCODE ]  ; then
	echo "Same exit code" >> "$ib".log 2>&1
    else
	echo "FAILED due to different exit code"
	continue
    fi

    if cmp "${ib}.out" "${ib}.vmout" >> "$ib".log 2>&1 ; then
	echo "Output matches exactly" >> "$ib".log 2>&1
    else
	echo "FAILED due to different output"
	continue
    fi

    e "$ib".log ${GCCCIL} -O3 -S ${GCCCIL_OPTIONS} $EXTRACIL1O3 "${ib}.exe" -o "${ib}.exe.O3.s"
    if [ $? -ne 0 ] ; then echo "FAILED compiling EXE to assembler with -O3" ; continue ; fi

    e "$ib".log ${GCCCIL} -O3 -v ${GCCCIL_OPTIONS} "${ib}.exe" -lm -o "${ib}.O3"
    if [ $? -ne 0 ] ; then echo "FAILED compiling EXE to executable with -O3" ; continue ; fi

    eoe "$ib".log "${ib}.O3.out" "./${ib}.O3"
    EXECUTABLE_O3_EXITCODE=$?
    echo "Executable with O3 output: <<EOF"  >> "$ib".log 2>&1
    cat "${ib}.O3.out"  >> "$ib".log 2>&1
    echo "EOF"  >> "$ib".log 2>&1
    echo "Exitcode: ${EXECUTABLE_O3_EXITCODE}" >> "$ib".log 2>&1

    if [ $VM_EXITCODE -eq $EXECUTABLE_O3_EXITCODE ]  ; then
	echo "Same exit code" >> "$ib".log 2>&1
    else
	echo "FAILED due to different exit code with -O3"
	continue
    fi

    if cmp "${ib}.out" "${ib}.O3.out" >> "$ib".log 2>&1 ; then
	echo "Output with -O3 matches exactly" >> "$ib".log 2>&1
    else
	echo "FAILED due to different output with -O3"
	continue
    fi

    e "$ib".log ${CIL32GCC} "$f" -O3 -S -o "${ib}.fO3.il"
    if [ $? -ne 0 ] ; then echo "FAILED compiling C to IL with -O3" ; continue ; fi

    e "$ib".log ${CIL32GCC} "$f" -O3 -o "${ib}.fO3.exe"
    if [ $? -ne 0 ] ; then echo "FAILED compiling C to EXE with -O3" ; continue ; fi

    eo "$ib".log "${ib}.fO3.exe.dis" ${DIS} "${ib}.fO3.exe" 
    if [ $? -ne 0 ] ; then echo "FAILED disassembling EXE from O3" ; continue ; fi



    e "$ib".log ${GCCCIL} -S "${ib}.fO3.exe" ${GCCCIL_OPTIONS} -o "${ib}.fO3.exe.s"
    if [ $? -ne 0 ] ; then echo "FAILED compiling -O3 EXE to assembler" ; continue ; fi

    e "$ib".log ${GCCCIL} -v ${GCCCIL_OPTIONS} "${ib}.fO3.exe" -lm -o "${ib}.fO3"
    if [ $? -ne 0 ] ; then echo "FAILED compiling -O3 EXE to executable" ; continue ; fi

    eoe "$ib".log "${ib}.fO3.out" "./${ib}.fO3"
    EXECUTABLE_FO3_EXITCODE=$?
    echo "Executable from O3 output: <<EOF"  >> "$ib".log 2>&1
    cat "${ib}.fO3.out"  >> "$ib".log 2>&1
    echo "EOF"  >> "$ib".log 2>&1
    echo "Exitcode: ${EXECUTABLE_FO3_EXITCODE}" >> "$ib".log 2>&1

    if [ $VM_EXITCODE -eq $EXECUTABLE_FO3_EXITCODE ]  ; then
	echo "Same exit code" >> "$ib".log 2>&1
    else
	echo "FAILED due to different exit code from -O3"
	continue
    fi

    if cmp "${ib}.fO3.out" "${ib}.O3.out" >> "$ib".log 2>&1 ; then
	echo "Output form -O3 matches exactly" >> "$ib".log 2>&1
    else
	echo "FAILED due to different from -O3 output"
	continue
    fi



    e "$ib".log ${GCCCIL} -O3 -S ${GCCCIL_OPTIONS} $EXTRACIL1O3 "${ib}.fO3.exe" -o "${ib}.fO3.exe.O3.s"
    if [ $? -ne 0 ] ; then echo "FAILED compiling -O3 EXE to assembler with -O3" ; continue ; fi

    e "$ib".log ${GCCCIL} -O3 -v ${GCCCIL_OPTIONS} "${ib}.fO3.exe" -lm -o "${ib}.fO3.O3"
    if [ $? -ne 0 ] ; then echo "FAILED compiling -O3 EXE to executable with -O3" ; continue ; fi

    eoe "$ib".log "${ib}.fO3.O3.out" "./${ib}.fO3.O3"
    EXECUTABLE_FO3_O3_EXITCODE=$?
    echo "Executable from O3 output with O3: <<EOF"  >> "$ib".log 2>&1
    cat "${ib}.fO3.O3.out"  >> "$ib".log 2>&1
    echo "EOF"  >> "$ib".log 2>&1
    echo "Exitcode: ${EXECUTABLE_FO3_O3_EXITCODE}" >> "$ib".log 2>&1

    if [ $VM_EXITCODE -eq $EXECUTABLE_FO3_O3_EXITCODE ]  ; then
	echo "Same exit code" >> "$ib".log 2>&1
    else
	echo "FAILED due to different exit code from -O3 with -O3"
	continue
    fi

    if cmp "${ib}.fO3.O3.out" "${ib}.O3.out" >> "$ib".log 2>&1 ; then
	echo "Output form -O3 with -O3 matches exactly" >> "$ib".log 2>&1
    else
	echo "FAILED due to different from -O3 output with -O3"
	continue
    fi

    echo "OK"
done
