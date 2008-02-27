#!/bin/sh

if test -z "$srcdir"; then
  srcdir=`echo "$0" | sed 's,[^/]*$,,'`
  test "$srcdir" = "$0" && srcdir=.
  test -z "$srcdir" && srcdir=.
  test "${VERBOSE+set}" != set && VERBOSE=1
fi
  VERBOSE=1
. $srcdir/defs

OMPNUM=1
if test "$enable_mpi" = "yes"; then
  if test -z "$MPIRUN"; then
    MPIRUN="mpirun -np 2"
  else
    MPIRUN="$MPIRUN $MPINP 2"
  fi
fi
if test "$enable_omp" = "yes"; then
  OMPNUM=2
fi

$MPIRUN $srcdir/test1 $srcdir/testmat.mtx 0 $srcdir/sol.txt $srcdir/res.txt -omp_num_threads $OMPNUM

if test "$enable_fortran" = "yes"; then
#  $MPIRUN $srcdir/test1f $srcdir/testmat.mtx 0 $srcdir/sol.txt $srcdir/res.txt -omp_num_threads $OMPNUM
    cd $srcdir; $MPIRUN ./test3f
fi

if test "$enable_saamg" = "yes"; then
  $MPIRUN $srcdir/test2 100 100 1 $srcdir/sol.txt $srcdir/res.txt -i cg -p saamg -omp_num_threads $OMPNUM
#  $MPIRUN $srcdir/test1 $srcdir/add32.mtx $srcdir/sol.txt $srcdir/res.txt -i bicgstab -p saamg -saamg_unsym true -omp_num_threads $OMPNUM
fi

if test "$enable_quad" = "yes"; then
  $MPIRUN $srcdir/test4 200 2.0 -precision double -omp_num_threads $OMPNUM
  $MPIRUN $srcdir/test4 200 2.0 -precision quad -omp_num_threads $OMPNUM
fi
