if WITH_HCAL

INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libhdate.la
ldadd= $(top_builddir)/src/libhdate.la -lm
MUDFLAP_OPTIONS = -g -fmudflap -funwind-tables -lmudflap -rdynamic

bin_PROGRAMS= hdate hcal

hdate_SOURCES = hdate.c
hdate_CFLAGS = $(MUDFLAP_OPTIONS)
hdate_LDFLAGS =
hdate_DEPENDENCIES = $(DEPS)
hdate_LDADD = $(ldadd)

hcal_SOURCES = hcal.c
hcal_CFLAGS = $(MUDFLAP_OPTIONS)
hcal_LDFLAGS =
hcal_DEPENDENCIES = $(DEPS)
hcal_LDADD = $(ldadd)

endif

libhdatedocdir = ${prefix}/share/doc/libhdate/examples/hcal
libhdatedoc_DATA = hcal.c hdate.c local_functions.c

EXTRA_DIST = $(libhdatedoc_DATA)