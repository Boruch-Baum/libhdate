if WITH_PYTHON

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.c *.so *wrap* *.py

all-am: _hdate.so
	

_hdate.so: hdate_wrap.c
	gcc -fpic -c hdate_wrap.c $(PY_CFLAGS)
	gcc -shared ../../src/*.o hdate_wrap.o -o _hdate.so

hdate_wrap.c: hdate.i
	swig -python hdate.i

install-data-local: _hdate.so
	$(mkinstalldirs) $(DESTDIR)/$(PY_MODULES_PATH)
	$(INSTALL_DATA) _hdate.so $(DESTDIR)$(PY_MODULES_PATH)/_hdate.so
	$(INSTALL_DATA) hdate.py $(DESTDIR)$(PY_MODULES_PATH)/hdate.py

endif
