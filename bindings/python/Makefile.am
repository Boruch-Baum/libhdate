if WITH_PYTHON

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_CFLAGS   = -fpic

DEPS = $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.cxx *.so *wrap* hdate.py


# ./configure failed with this following snippet, and with
# AM_PATH_PYTHON set in configure.ac to various values, and even with
# PC_INIT(2.4, 2.9)
#
ACLOCAL_AMFLAGS = -I m4

hdate.py hdate_wrap.cxx: hdate.i
	$(SWIG) $(AX_SWIG_PYTHON_OPT) -I$(top_srcdir)/src -o $@ $<

python_PYTHON = $(builddir)/hdate.py

pyexec_LTLIBRARIES = _hdate.la
_hdate_la_SOURCES = hdate_wrap.cxx
_hdate_la_LDFLAGS = -module ${PYTHON_LDFLAGS}  -I$(top_srcdir)/src
_hdate_la_CPPFLAGS = ${PYTHON_CPPFLAGS}  -I$(top_srcdir)/src
# not sure about this next line
_hdate_la_LIBADD = ../../src/.libs/libhdate.so

BUILT_SOURCES = hdate.py

#all-am: _hdate.so
#
#_hdate.so: hdate_wrap.cxx
#	g++ -c hdate_wrap.cxx $(CPPFLAGS) $(CFLAGS) $(PY_CFLAGS) -I$(top_srcdir)/src
#	g++ -shared hdate_wrap.o ../../src/.libs/libhdate.so -o _hdate.so $(LDFLAGS)
#
#hdate_wrap.cxx: hdate.i
#	swig -python -c++ hdate.i
#
#install-data-local: _hdate.so
#	$(mkinstalldirs) $(DESTDIR)$(PY_MODULES_PATH)
#	$(INSTALL_DATA) _hdate.so $(DESTDIR)$(PY_MODULES_PATH)/_hdate.so
#	$(INSTALL_DATA) hdate.py $(DESTDIR)$(PY_MODULES_PATH)/hdate.py

endif

EXTRA_DIST = hdate.i setup.py hdate.py
