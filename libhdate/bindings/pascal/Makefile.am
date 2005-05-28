
EXTRA_DIST = Package.fpc hdate.pp hdate_class.pp fpc_README changelog

if WITH_PASCAL

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.ppu

all-am: hdate.o hdate_class.o

hdate.o: hdate.pp ctypes.o
	fpc hdate.pp

hdate_class.o: hdate_class.pp hdate.o hcal.o
	fpc hdate_obj.pp

install-data-local:hdate_class.o hdate.o Package.fpc
	$(mkinstalldirs) $(DESTDIR)$(PA_MODULES_PATH)/hdate
	$(INSTALL_DATA) hdate.o $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate.o
	$(INSTALL_DATA) hdate_obj.o $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate_class.o
	$(INSTALL_DATA) hdate.ppu $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate.ppu
	$(INSTALL_DATA) hdate_obj.ppu $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate_class.ppu
	$(INSTALL_DATA) fpc_README $(DESTDIR)$(PA_MODULES_PATH)/hdate/fpc_README
	$(INSTALL_DATA) Package.fpc $(DESTDIR)$(PA_MODULES_PATH)/hdate/Package.fpc
endif
