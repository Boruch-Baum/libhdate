
EXTRA_DIST = hdate.i

if WITH_PERL

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.cxx *.so *wrap* *.pm

all-am: hdate.so
	

hdate.so: hdate_wrap.cxx
	g++ -fpic -c hdate_wrap.cxx $(PE_CFLAGS) -I$(top_srcdir)/src
	g++ -shared ../../src/*.o hdate_wrap.o -o hdate.so

hdate_wrap.cxx: hdate.i
	swig -perl5 -c++ hdate.i

install-data-local: hdate.so
	$(mkinstalldirs) $(DESTDIR)$(PE_MODULES_PATH)
	$(INSTALL_DATA) hdate.so $(DESTDIR)$(PE_MODULES_PATH)/hdate.so
	$(INSTALL_DATA) hdate.pm $(DESTDIR)$(PE_MODULES_PATH)/hdate.pm

endif
