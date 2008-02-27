SUBDIRS = matrix vector matvec precon solver system precision
if ENABLE_FORTRAN
  SUBDIRS += fortran
endif
if ENABLE_SAAMG
  SUBDIRS += fortran
  SUBDIRS += fortran/amg
endif

lib_LTLIBRARIES  = liblis.la
liblis_la_LIBADD = matrix/libmatrix.la \
                   vector/libvector.la \
                   matvec/libmatvec.la \
                   precon/libprecon.la \
                   solver/libsolver.la \
                   system/libsystem.la \
                   precision/libprecision.la 
if ENABLE_FORTRAN
  liblis_la_LIBADD += fortran/libfortran.la
endif
if ENABLE_SAAMG
  liblis_la_LIBADD += fortran/amg/libsaamg.la
  liblis_la_LIBADD += fortran/libfortran.la
endif
liblis_la_SOURCES = 
