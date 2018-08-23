if WITH_HCAL

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS   = -O1 -fPIC -fPIE -pie \
              -D_FORTIFY_SOURCE=2 -fstack-protector --param ssp-buffer-size=4 \
              -Wall -Wformat-security -Werror=format-security
# CFLAGS      -g -ggdb3 -gstabs+ -Wextra
DEPS  = $(top_builddir)/src/libhdate.la
LDADD = $(top_builddir)/src/libhdate.la -lm
AM_LDFLAGS  = -z relro -z now

bin_PROGRAMS = hdate hcal

hdate_SOURCES = hdate.c local_functions.c custom_days.c timezone_functions.c
# hdate_CFLAGS =
# hdate_LDFLAGS =
# hdate_DEPENDENCIES = $(DEPS)
# hdate_LDADD =

hcal_SOURCES = hcal.c local_functions.c custom_days.c timezone_functions.c
# hcal_CFLAGS =
# hcal_LDFLAGS =
# hcal_DEPENDENCIES = $(DEPS)
# hcal_LDADD =

endif

libhdatedocdir = ${prefix}/share/doc/libhdate/examples/hcal
libhdatedoc_DATA = hcal.c hdate.c local_functions.c custom_days.c timezone_functions.c

#EXTRA_DIST = $(libhdatedoc_DATA)
