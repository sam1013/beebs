#!/bin/bash

source ../../../config.common

# To record instruction histograms when executing benchmarks, set -DTM_BENCH=1.
# This will terminate benchmark program immediately when executing stop_trigger without
# doing the verification step. 
# In order to verify the benchmark execution results, set -DTM_BENCH=0. This will return
# the benchmark status code to the commandline.
CFLAGS="-I/opt/riscv/${RISCV_XLEN}/include/spike -march=rv${RISCV_XLEN}g -DTM_BENCH=1 -O1 -g"

export EXCLUDED_BENCHMARKS=crc32,ctl-string,ctl-vector,cubic,dtoa,fasta,frac,levenshtein,matmult-float,mergesort,miniz,nbody,qrduino,rijndael,slre,stringsearch1,st,stb_perlin,trio-snprintf,trio-sscanf,wikisort,whetstone

MALLOC_BENCHMARKS="ctl-stack dijkstra huffbench sglib-dllist sglib-hashtable sglib-listinsertsort sglib-listsort sglib-rbtree"

#all benchmarks using malloc. Not all are compiling, since we miss stdlib
#ctl,ctl-string,ctl-stack,dijkstra,dtoa,fasta,huffbench,miniz,qrduino,sglib-dllist,sglib-hashtable,sglib-listinsertsort,sglib-listsort,sglib-rbtree,trio,wikisort

if ! [[ -f bench.inc ]]; then

# Determine list of compiling benchmarks
make clean

autoreconf

while true; do
  export EXCLUDED_BENCHMARKS
  ./configure --host=riscv${RISCV_XLEN}-unknown-elf --with-chip=generic --with-board=none CFLAGS="${CFLAGS}" &> /dev/null || exit $?
  make &> out.txt
  # Compilation failed. Determine failing benchmark from error log and add it to the exclude list
  if [[ "$?" -ne "0" ]]; then
    failedtc=$(awk '/Error/ { print gensub(/.*\*\*\*\s\[(.*)\]\sError.*/, "\\1", "g", $0); exit 0;}' out.txt)
    EXCLUDED_BENCHMARKS=$EXCLUDED_BENCHMARKS,$failedtc
    echo "Excluded: $EXCLUDED_BENCHMARKS"
    rm -f out.txt
  else
    rm -f out.txt
    break
  fi
done
fi

# Fail on error
set -e

# Extract list of compiling benchmarks from (autogenerated) Makefile
sed -e 's/^BENCHMARKS = \(.*\)$/export BENCHMARKS="\1"/;t;d' Makefile > bench.inc

source bench.inc

# Re-generate bench.pyinc
echo "beebs_tests = [" > bench.pyinc
for b in $BENCHMARKS; do
  echo "\"$b\", " >> bench.pyinc
done
echo "]" >> bench.pyinc
echo "" >> bench.pyinc
echo "beebs_hitests = [" >> bench.pyinc
for b in $MALLOC_BENCHMARKS; do
  echo "\"$b\", " >> bench.pyinc
done
echo "]" >> bench.pyinc

if true; then
# Compile benchmarks without SI
make clean
./configure --host=riscv${RISCV_XLEN}-unknown-elf --with-chip=generic --with-board=none CFLAGS="${CFLAGS}" &> /dev/null
make
for b in $BENCHMARKS; do
  echo $b
  cp src/$b/$b ../results/$b.elf
done

# Compile benchmarks with SI (stack interleaving) and CH (code hardening)
for b in $BENCHMARKS; do
  echo $b-SI
  make clean
  make -C src/$b si
  cp src/$b/$b ../results/$b-SI.elf
  echo $b-CH
  make clean
  make -C src/$b ch
  cp src/$b/$b ../results/$b-CH.elf
done

fi

# Compile benchmarks without SI but with HI (heap interleaving)
make clean
./configure --host=riscv${RISCV_XLEN}-unknown-elf --with-chip=generic --with-board=none CFLAGS="${CFLAGS} -DHEAP_INTERLEAVING=1" &> /dev/null
for b in $MALLOC_BENCHMARKS; do
  echo $b-HI
  make -C src/$b all
  cp src/$b/$b ../results/$b-HI.elf
done
