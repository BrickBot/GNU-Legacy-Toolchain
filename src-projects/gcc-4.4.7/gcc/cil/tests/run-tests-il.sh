#!/bin/bash

if [ $# -eq 0 ] ; then
    TESTS="bitfieldx.il mp3dec.il volatile.il"
else
    TESTS="$@"
fi
TESTSDIR="$(dirname $0)"


GCCCIL_PREFIX="/home/ricardo/mine/install"
GCCCIL="${GCCCIL_PREFIX}/bin/gcil"
#GCCCIL_OPTIONS=" -fuse-mono-runtime"
GCCCIL_OPTIONS=""
VM='mono'
DIS='monodis'
ILASM='ilasm.pnet'

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

    e "$ib".log ${ILASM} -e -o ${ib}.exe "$f" 
    if [ $? -ne 0 ] ; then echo "FAILED compiling IL to EXE" ; continue ; fi

    eo "$ib".log ${ib}.exe.dis ${DIS} ${ib}.exe 
    if [ $? -ne 0 ] ; then echo "FAILED disassembling EXE" ; continue ; fi

    e "$ib".log ${GCCCIL} -S "${ib}.exe" ${GCCCIL_OPTIONS} -o "${ib}.exe.s"
    if [ $? -ne 0 ] ; then echo "FAILED compiling EXE to assembler" ; continue ; fi


    e "$ib".log ${GCCCIL} -v ${GCCCIL_OPTIONS} "${ib}.exe" -o "${ib}"
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

    e "$ib".log ${GCCCIL} -O3 -S $EXTRACIL1O3 "${ib}.exe" ${GCCCIL_OPTIONS} -o "${ib}.exe.O3.s"
    if [ $? -ne 0 ] ; then echo "FAILED compiling EXE to assembler with -O3" ; continue ; fi

    e "$ib".log ${GCCCIL} -O3 -v ${GCCCIL_OPTIONS} "${ib}.exe" -o "${ib}.O3"
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

    echo "OK"
done
