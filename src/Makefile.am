libhdate_la_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libhdate.la

libhdate_la_SOURCES = \
	deprecated.c\
	hdate_strings.c\
	hdate_julian.c\
	hdate_holyday.c\
	hdate_parasha.c\
	hdate_parse_date.c\
	hdate_sun_time.c\
	zdump3.c\
	zdump3.h\
	hdate.h\
	support.h

libhdate_la_LDFLAGS = -version-info 0:0:0
# the ld flag `version-info' caused me much grief when returning to
# this project in 2015. It had prior been set to $(VERSION_INFO), which had been set in file `configure.in' ... well here are the two lines ...
#
## dnl libtool library numbering
# VERSION_INFO=$LIBHDATE_MAJOR_VERSION:$LIBHDATE_MINOR_VERSION:0
#
# -- but that's wrong ! At least, now it is. Refer to libtool manual,
# section 7.3 `Updating library version information'. So I attempted,
# and failed to otherwise define custom variables in that file in a
# way to nicely support this feature. The closest I got was the
# following, but all my attempts resulted in the variable being
# replace with nil.
#
# AC_DEFINE([INTERFACE_CURRENT],[0])
# AC_DEFINE([INTERFACE_REVISION],[0])
# AC_DEFINE([INTERFACE_AGE],[0])
# AC_DEFINE([INTERFACE_VERSION],[${INTERFACE_CURRENT}:${INTERFACE_REVISION}:${INTERFACE_AGE}])


libhdate_la_LIBADD = 

include_HEADERS = hdate.h hdatepp.h

