## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = intl po src examples bindings debian

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libhdate.pc

libhdatedocdir = ${prefix}/doc/hdate
libhdatedoc_DATA = \
	README\
	hdate_README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	TODO\
	USE\
	ABOUT-NLS

EXTRA_DIST = $(libhdatedoc_DATA) libhdate.pc.in libhdate.spec libhdate.spec.in
