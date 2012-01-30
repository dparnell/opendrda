include_HEADERS = connectparams.h
DRDADIR         =    ../libdrda
DRDASOURCES     =    ddmwrite.c drda.c user.c ddmread.c debug.c fdoca.c handle.c util.c convert.c
bin_PROGRAMS	=	unittest
lib_LTLIBRARIES	=	libdrdaodbc.la
INCLUDES	=	-I ../../include `glib-config --cflags`
libdrdaodbc_la_SOURCES=	odbc.c connectparams.c
libdrdaodbc_la_LIBADD=	$(patsubst %, $(DRDADIR)/%, \
				$(patsubst %.c, %.lo, $(DRDASOURCES))) -lglib
unittest_LDADD	=	libdrdaodbc.la ../libdrda/libdrda.la -lglib

## Need blank statement to avoid compiling odbc.c
odbc: $(EXTRA_LTLIBRARIES)
	@echo ''

