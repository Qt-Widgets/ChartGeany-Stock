#
# OpenSSL/crypto/rc5/Makefile
#

DIR=	rc5
TOP=	../..
CC=	cc
CPP=	$(CC) -E
INCLUDES=
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r

RC5_ENC=		rc5_enc.o

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)
AFLAGS= $(ASFLAGS)

GENERAL=Makefile
TEST=rc5test.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=rc5_skey.c rc5_ecb.c rc5_enc.c rc5cfb64.c rc5ofb64.c 
LIBOBJ=rc5_skey.o rc5_ecb.o $(RC5_ENC) rc5cfb64.o rc5ofb64.o

SRC= $(LIBSRC)

EXHEADER= rc5.h
HEADER=	rc5_locl.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

rc5-586.s: asm/rc5-586.pl ../perlasm/x86asm.pl ../perlasm/cbc.pl
	$(PERL) asm/rc5-586.pl $(PERLASM_SCHEME) $(CFLAGS) > $@

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
	rm -f *.s *.o *.obj lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

rc5_ecb.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
rc5_ecb.o: ../../include/openssl/rc5.h c:\mingw\include\_mingw.h
rc5_ecb.o: c:\mingw\include\stdlib.h
rc5_ecb.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h rc5_ecb.c rc5_locl.h
rc5_enc.o: ../../include/openssl/opensslconf.h ../../include/openssl/rc5.h
rc5_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\stdio.h
rc5_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\sys\types.h
rc5_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
rc5_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h rc5_enc.c rc5_locl.h
rc5_skey.o: ../../include/openssl/opensslconf.h ../../include/openssl/rc5.h
rc5_skey.o: c:\mingw\include\_mingw.h c:\mingw\include\stdlib.h
rc5_skey.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h rc5_locl.h
rc5_skey.o: rc5_skey.c
rc5cfb64.o: ../../include/openssl/opensslconf.h ../../include/openssl/rc5.h
rc5cfb64.o: c:\mingw\include\_mingw.h c:\mingw\include\stdlib.h
rc5cfb64.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h rc5_locl.h
rc5cfb64.o: rc5cfb64.c
rc5ofb64.o: ../../include/openssl/opensslconf.h ../../include/openssl/rc5.h
rc5ofb64.o: c:\mingw\include\_mingw.h c:\mingw\include\stdlib.h
rc5ofb64.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h rc5_locl.h
rc5ofb64.o: rc5ofb64.c
