if WITH_HCAL

# For debugging: CFLAGS=" -g -ggdb3 -gstabs+ -Wextra " make

# Consider moving to the root makefile the defintions
# for AM_CPPFLAGS, AM_CFLAGS, AM_LDFLAGS

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS   = -O1 -fPIC -fPIE -pie \
              -D_FORTIFY_SOURCE=2 -fstack-protector --param ssp-buffer-size=4 \
              -Wall -Wformat-security -Werror=format-security
AM_LDFLAGS  = -z relro -z now

DEPS  = $(top_builddir)/src/libhdate.la
LDADD = $(top_builddir)/src/libhdate.la -lm

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

libhdatedocdir = ${docdir}/examples/hcal
libhdatedoc_DATA = hcal.c hdate.c local_functions.c custom_days.c timezone_functions.c

#EXTRA_DIST = $(libhdatedoc_DATA)
