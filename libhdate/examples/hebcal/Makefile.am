if WITH_EXAMPLES

INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libhdate.la
ldadd= $(top_builddir)/src/libhdate.la -lm

bin_PROGRAMS = hdate_hebcal
hdate_hebcal_SOURCES = hdate_hebcal.c
hdate_hebcal_LDFLAGS =
hdate_hebcal_DEPENDENCIES = $(DEPS)
hdate_hebcal_LDADD = $(ldadd)

endif
