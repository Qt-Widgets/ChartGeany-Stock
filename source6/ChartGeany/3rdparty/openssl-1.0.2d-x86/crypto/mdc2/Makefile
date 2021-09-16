#
# OpenSSL/crypto/mdc2/Makefile
#

DIR=	mdc2
TOP=	../..
CC=	cc
INCLUDES=
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

CFLAGS= $(INCLUDES) $(CFLAG)

GENERAL=Makefile
TEST= mdc2test.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=mdc2dgst.c mdc2_one.c
LIBOBJ=mdc2dgst.o mdc2_one.o

SRC= $(LIBSRC)

EXHEADER= mdc2.h
HEADER=	$(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
	@$(PERL) $(TOP)/util/mklink.pl ../../apps $(APPS)

install:
	@[ -n "$(INSTALLTOP)" ] # should be set by top Makefile...
	@headerlist="$(EXHEADER)"; for i in $$headerlist ; \
	do  \
	(cp $$i $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i; \
	chmod 644 $(INSTALL_PREFIX)$(INSTALLTOP)/include/openssl/$$i ); \
	done;

tags:
	ctags $(SRC)

tests:

lint:
	lint -DLINT $(INCLUDES) $(SRC)>fluff

update: depend

depend:
	@[ -n "$(MAKEDEPEND)" ] # should be set by upper Makefile...
	$(MAKEDEPEND) -- $(CFLAG) $(INCLUDES) $(DEPFLAG) -- $(PROGS) $(LIBSRC)

dclean:
	$(PERL) -pe 'if (/^# DO NOT DELETE THIS LINE/) {print; exit(0);}' $(MAKEFILE) >Makefile.new
	mv -f Makefile.new $(MAKEFILE)

clean:
	rm -f *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

mdc2_one.o: ../../e_os.h ../../include/openssl/bio.h
mdc2_one.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
mdc2_one.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
mdc2_one.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
mdc2_one.o: ../../include/openssl/lhash.h ../../include/openssl/mdc2.h
mdc2_one.o: ../../include/openssl/opensslconf.h
mdc2_one.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
mdc2_one.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
mdc2_one.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
mdc2_one.o: ../../include/openssl/ui_compat.h ../../ms/applink.c
mdc2_one.o: ../../ms/uplink.h ../cryptlib.h c:\mingw\include\_mingw.h
mdc2_one.o: c:\mingw\include\basetsd.h c:\mingw\include\errno.h
mdc2_one.o: c:\mingw\include\fcntl.h c:\mingw\include\io.h
mdc2_one.o: c:\mingw\include\malloc.h c:\mingw\include\poppack.h
mdc2_one.o: c:\mingw\include\pshpack4.h c:\mingw\include\stdio.h
mdc2_one.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
mdc2_one.o: c:\mingw\include\sys\types.h c:\mingw\include\winbase.h
mdc2_one.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
mdc2_one.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
mdc2_one.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
mdc2_one.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
mdc2_one.o: c:\mingw\include\winreg.h c:\mingw\include\winsock2.h
mdc2_one.o: c:\mingw\include\winsvc.h c:\mingw\include\winuser.h
mdc2_one.o: c:\mingw\include\winver.h c:\mingw\include\ws2tcpip.h
mdc2_one.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
mdc2_one.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h mdc2_one.c
mdc2dgst.o: ../../include/openssl/crypto.h ../../include/openssl/des.h
mdc2dgst.o: ../../include/openssl/des_old.h ../../include/openssl/e_os2.h
mdc2dgst.o: ../../include/openssl/mdc2.h ../../include/openssl/opensslconf.h
mdc2dgst.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
mdc2dgst.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
mdc2dgst.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
mdc2dgst.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
mdc2dgst.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
mdc2dgst.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
mdc2dgst.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
mdc2dgst.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h mdc2dgst.c
