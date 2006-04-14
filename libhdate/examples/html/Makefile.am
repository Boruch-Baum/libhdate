EXTRA_DIST = hdate_reading_html.c hdate_holydays_html.c

if WITH_EXAMPLES

INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libhdate.la
ldadd= $(top_builddir)/src/libhdate.la -lm

bin_PROGRAMS = hdate_reading_html hdate_holydays_html

hdate_reading_html_SOURCES = hdate_reading_html.c
hdate_reading_html_LDFLAGS =
hdate_reading_html_DEPENDENCIES = $(DEPS)
hdate_reading_html_LDADD = $(ldadd)

hdate_holydays_html_SOURCES = hdate_holydays_html.c
hdate_holydays_html_LDFLAGS =
hdate_holydays_html_DEPENDENCIES = $(DEPS)
hdate_holydays_html_LDADD = $(ldadd)

endif

libhdatedocdir = ${prefix}/share/doc/libhdate/examples/html
libhdatedoc_DATA = hdate_holydays_html.c hdate_reading_html.c
