LDADD = -L$(top_builddir)/src -llis
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/include
AM_FFLAGS = -I$(top_builddir) -I$(top_srcdir)/include
AM_LDFLAGS = $(AMDEFS)

EXTRA_DIST = test1.icproj test1.vcproj \
             test1f.vfproj \
             test2.icproj test2.vcproj \
             test3.icproj test3.vcproj \
             test3f.vfproj \
             test4.icproj test4.vcproj \
             defs test.sh testmat.mtx

noinst_PROGRAMS = test1 test2 test3 test4 test5
if ENABLE_FORTRAN
  noinst_PROGRAMS += test1f test3f
endif
#if ENABLE_DEVELOP
#  noinst_PROGRAMS += matvect matvect_quad matvec_fem solver_compare
#endif
test1_SOURCES  = test1.c
test2_SOURCES  = test2.c
test3_SOURCES  = test3.c
test4_SOURCES  = test4.c
test5_SOURCES  = test5.c
if ENABLE_FORTRAN
  test1f_SOURCES  = test1f.F
  test3f_SOURCES  = test3f.F
endif
#if ENABLE_DEVELOP
#  matvect_SOURCES  = matvect.c
#  matvect_quad_SOURCES  = matvect_quad.c
#  matvec_fem_SOURCES  = matvec_fem.c
#  solver_compare_SOURCES  = solver_compare.c
#endif

PPF77COMPILE = $(F77) $(AMDEFS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_FFLAGS) $(FFLAGS)
LTPPF77COMPILE = $(LIBTOOL) --mode=compile --tag=F77 $(F77) $(AMDEFS) $(DEFS) \
	$(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) \
	$(AM_FFLAGS) $(FFLAGS)
COMPILE = $(CC) $(AMDEFS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
LTCOMPILE = $(LIBTOOL) --mode=compile --tag=CC $(CC) $(AMDEFS) $(DEFS) \
	$(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) \
	$(AM_CFLAGS) $(CFLAGS)

if ENABLE_SAAMG
  LINK = $(LIBTOOL) --mode=link $(FC) \
	$(AM_LDFLAGS) $(LDFLAGS) $(FCLDFLAGS) -o $@ $(LIBS) $(CLIBS)
  F77LINK = $(LIBTOOL) --mode=link $(FC) \
	$(AM_LDFLAGS) $(LDFLAGS) -o $@ $(LIBS) $(CLIBS)
else
if ENABLE_FORTRAN
    LINK = $(LIBTOOL) --mode=link $(FC) \
	$(AM_LDFLAGS) $(LDFLAGS) $(FCLDFLAGS) -o $@ $(LIBS) $(CLIBS)
    F77LINK = $(LIBTOOL) --mode=link $(F77) \
	$(AM_LDFLAGS) $(LDFLAGS) -o $@ $(LIBS) $(CLIBS)
else
    LINK = $(LIBTOOL) --mode=link $(CC) $(AM_LDFLAGS) $(LDFLAGS) -o $@ $(LIBS)
endif
endif
CLEANFILES = *.il


TESTS_ENVIRONMENT = top_builddir=$(top_builddir) enable_mpi=$(enable_mpi) \
                    enable_omp=$(enable_omp) enable_saamg=$(enable_saamg) \
                    enable_fortran=$(enable_fortran) enable_quad=$(enable_quad) \
                    MPIRUN=$(MPIRUN) MPINP=$(MPINP)

TESTS = test.sh

