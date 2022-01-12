
EXTRA_DIST = hdate.i

# 2018-08 The following is insufficient to resolve a swig error
# AM_hdate.so_CFLAGS = -I$(dirname $(locate 'ruby/config.h' |head -n1))

if WITH_RUBY

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.cxx *.so *wrap*

all-am: hdate.so

hdate.so: hdate_wrap.cxx
	g++ -fpic -fno-strict-aliasing -c hdate_wrap.cxx $(RU_CFLAGS)
	gcc -shared hdate_wrap.o ../../src/.libs/libhdate.so -rdynamic -Wl,-export-dynamic -lruby$(RU_VERSION) -lpthread -ldl -lcrypt -lm -lc -lstdc++ -o hdate.so


hdate_wrap.cxx: hdate.i
	swig -c++ -ruby hdate.i

install-data-local: hdate.so
	$(mkinstalldirs) $(DESTDIR)$(RU_MODULES_PATH)
	$(INSTALL_DATA) hdate.so $(DESTDIR)$(RU_MODULES_PATH)/hdate.so

endif
