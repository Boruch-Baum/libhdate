
EXTRA_DIST = Package.fpc ctypes.pp hdate.pp hdate_obj.pp fpc_README

if WITH_PASCAL

DEPS= $(top_builddir)/src/libhdate.la

CLEANFILES = *.o *.ppu

all-am: ctypes.o hdate.o hdate_obj.o

ctypes.o: ctypes.pp
	fpc ctypes.pp

hdate.o: hdate.pp ctypes.o
	fpc hdate.pp

hdate_obj.o: hdate_obj.pp hdate.o
	fpc hdate_obj.pp

install-data-local:hdate_obj.o hdate.o ctypes.o Package.fpc
	$(mkinstalldirs) $(DESTDIR)$(PA_MODULES_PATH)/hdate
	$(INSTALL_DATA) hdate.o $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate.o
	$(INSTALL_DATA) hdate_obj.o $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate_obj.o
	$(INSTALL_DATA) ctypes.o $(DESTDIR)$(PA_MODULES_PATH)/hdate/ctypes.o
	$(INSTALL_DATA) hdate.ppu $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate.ppu
	$(INSTALL_DATA) hdate_obj.ppu $(DESTDIR)$(PA_MODULES_PATH)/hdate/hdate_obj.ppu
	$(INSTALL_DATA) ctypes.ppu $(DESTDIR)$(PA_MODULES_PATH)/hdate/ctypes.ppu
	$(INSTALL_DATA) fpc_README $(DESTDIR)$(PA_MODULES_PATH)/hdate/fpc_README
	$(INSTALL_DATA) Package.fpc $(DESTDIR)$(PA_MODULES_PATH)/hdate/Package.fpc
endif
