if WITH_HCAL

INCLUDES=-I$(top_srcdir)/src

DEPS= $(top_builddir)/src/libhdate.la
ldadd= $(top_builddir)/src/libhdate.la -lm

bin_PROGRAMS= hdate hcal

hdate_SOURCES = hdate.c local_functions.c custom_days.c timezone_functions.c zdump3.c
hdate_CFLAGS = -Wall -Wformat -Wformat-security -Werror=format-security -D_FORTIFY_SOURCE=2 -fstack-protector --param ssp-buffer-size=4 -fPIC -fPIE -pie -g -ggdb3 -gstabs+ 
hdate_LDFLAGS = -z relro -z now
hdate_DEPENDENCIES = $(DEPS)
hdate_LDADD = $(ldadd)

hcal_SOURCES = hcal.c local_functions.c custom_days.c timezone_functions.c zdump3.c
hcal_CFLAGS = -Wall -Wformat -Wformat-security -Werror=format-security -D_FORTIFY_SOURCE=2 -fstack-protector --param ssp-buffer-size=4 -fPIC -fPIE -pie -g -ggdb3 -gstabs+
hcal_LDFLAGS =  -z relro -z now
hcal_DEPENDENCIES = $(DEPS)
hcal_LDADD = $(ldadd)

endif

libhdatedocdir = ${prefix}/share/doc/libhdate/examples/hcal
libhdatedoc_DATA = hcal.c hdate.c local_functions.c

EXTRA_DIST = $(libhdatedoc_DATA)


## For debugging
## CFLAGS +=  $(CFLAGS) -ggdb3 -gstabs+
## For real
## CFLAGS += -O1
