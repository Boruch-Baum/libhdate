EXTRA_DIST = hdate.1

if WITH_HCAL

INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libhdate.la
ldadd= $(top_builddir)/src/libhdate.la -lm

bin_PROGRAMS= hdate hcal

hdate_SOURCES = hdate.c
hdate_LDFLAGS =
hdate_DEPENDENCIES = $(DEPS)
hdate_LDADD = $(ldadd)

hcal_SOURCES = hcal.c
hcal_LDFLAGS =
hcal_DEPENDENCIES = $(DEPS)
hcal_LDADD = $(ldadd)

man_MANS = hdate.1

endif
