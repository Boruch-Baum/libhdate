EXTRA_DIST = hdate_reading_ical.c hdate_holydays_ical.c

if WITH_EXAMPLES

INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libhdate.la
ldadd= $(top_builddir)/src/libhdate.la -lm

bin_PROGRAMS = hdate_reading_ical hdate_holydays_ical

hdate_reading_ical_SOURCES = hdate_reading_ical.c
hdate_reading_ical_LDFLAGS =
hdate_reading_ical_DEPENDENCIES = $(DEPS)
hdate_reading_ical_LDADD = $(ldadd)

hdate_holydays_ical_SOURCES = hdate_holydays_ical.c
hdate_holydays_ical_LDFLAGS =
hdate_holydays_ical_DEPENDENCIES = $(DEPS)
hdate_holydays_ical_LDADD = $(ldadd)

endif
