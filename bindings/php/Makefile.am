
EXTRA_DIST = hdate.i hdate.ini

if WITH_PHP

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.cpp *.h *.so *.php *wrap*

all-am: hdate.so

hdate.so: hdate_wrap.cpp
	g++ -I$(top_srcdir)/src `php-config5 --includes` -fpic -c hdate_wrap.cpp
	gcc -shared ../../src/.libs/libhdate.so hdate_wrap.o -lstdc++ -o hdate.so

# Note that lstdc++ is for 32-bit and requires package 'g++-multilib'

hdate_wrap.cpp: hdate.i
	alias php=php5 ; swig -php5 -c++ hdate.i

install-data-local: hdate.so hdate.ini hdate.php
	$(mkinstalldirs) $(DESTDIR)$(PH_MODULES_PATH)
	$(INSTALL_DATA) hdate.so $(DESTDIR)$(PH_MODULES_PATH)/hdate.so
	$(mkinstalldirs) $(DESTDIR)$(PH_CONFIG_PATH)
	$(INSTALL_DATA) hdate.ini $(DESTDIR)$(PH_CONFIG_PATH)/hdate.ini
	$(mkinstalldirs) $(DESTDIR)$(PH_INCLUDE_PATH)
	$(INSTALL_DATA) hdate.php $(DESTDIR)$(PH_INCLUDE_PATH)/hdate.php
endif
