## Process this file with automake to produce Makefile.in

PACKAGE_NAME = libhdate
PACKAGE_STRING = library for Hebrew date information
PACKAGE_TARNAME = 
PACKAGE_URL = libhdate.sourceforge.net
PACKAGE_VERSION = 1.8
VERSION = 1.8

SUBDIRS = po docs src examples bindings

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libhdate.pc

libhdatedocdir = ${prefix}/share/doc/libhdate
libhdatedoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	USE \
	ABOUT-NLS 

EXTRA_DIST = $(libhdatedoc_DATA) libhdate.pc.in

ACLOCAL_AMFLAGS = -I m4

CFLAGS = -Wall -Wformat -Wformat-security -Werror=format-security -D_FORTIFY_SOURCE=2 -fstack-protector --param ssp-buffer-size=4 -fPIE -pie
LDFLAGS = -z relro -z now

## For debugging
## CFLAGS =  $(CFLAGS) -ggdb3 -gstabs+ -p -pg
CFLAGS += -ggdb3 -gstabs+ -p -pg
## For real
## CFLAGS += -O1
