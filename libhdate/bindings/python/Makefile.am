if WITH_PYTHON

EXTRA_DIST = foo.i

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.c *.so *wrap* *.py

all-am: _hdate.so
	

_hdate.so: hdate_wrap.c
	g++ -fpic -c hdate_wrap.c $(PY_CFLAGS)
	g++ -shared ../../src/*.o hdate_wrap.o -o _hdate.so

hdate_wrap.c: hdate.i
	swig -python -c++ hdate.i

install-data-local: _hdate.so
	$(mkinstalldirs) $(DESTDIR)/$(PY_MODULES_PATH)
	$(INSTALL_DATA) _hdate.so $(DESTDIR)$(PY_MODULES_PATH)/_hdate.so
	$(INSTALL_DATA) hdate.py $(DESTDIR)$(PY_MODULES_PATH)/hdate.py

endif
