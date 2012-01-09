## Process this file with automake to produce Makefile.in

PACKAGE_NAME = libhdate
PACKAGE_STRING = library for Hebrew date information
PACKAGE_TARNAME = 
PACKAGE_URL = libhdate.sourceforge.net
PACKAGE_VERSION = 1.6
VERSION = 1.6

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
