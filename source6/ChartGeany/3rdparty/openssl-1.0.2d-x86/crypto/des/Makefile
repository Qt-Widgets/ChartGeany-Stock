#
# OpenSSL/crypto/des/Makefile
#

DIR=	des
TOP=	../..
CC=	cc
CPP=	$(CC) -E
INCLUDES=-I$(TOP) -I../../include
CFLAG=-g
MAKEFILE=	Makefile
AR=		ar r
RANLIB=		ranlib
DES_ENC=	des_enc.o fcrypt_b.o

CFLAGS= $(INCLUDES) $(CFLAG)
ASFLAGS= $(INCLUDES) $(ASFLAG)
AFLAGS= $(ASFLAGS)

GENERAL=Makefile
TEST=destest.c
APPS=

LIB=$(TOP)/libcrypto.a
LIBSRC=	cbc_cksm.c cbc_enc.c  cfb64enc.c cfb_enc.c  \
	ecb3_enc.c ecb_enc.c  enc_read.c enc_writ.c \
	fcrypt.c ofb64enc.c ofb_enc.c  pcbc_enc.c \
	qud_cksm.c rand_key.c rpc_enc.c  set_key.c  \
	des_enc.c fcrypt_b.c \
	xcbc_enc.c \
	str2key.c  cfb64ede.c ofb64ede.c ede_cbcm_enc.c des_old.c des_old2.c \
	read2pwd.c

LIBOBJ= set_key.o  ecb_enc.o  cbc_enc.o \
	ecb3_enc.o cfb64enc.o cfb64ede.o cfb_enc.o  ofb64ede.o \
	enc_read.o enc_writ.o ofb64enc.o \
	ofb_enc.o  str2key.o  pcbc_enc.o qud_cksm.o rand_key.o \
	${DES_ENC} \
	fcrypt.o xcbc_enc.o rpc_enc.o  cbc_cksm.o \
	ede_cbcm_enc.o des_old.o des_old2.o read2pwd.o

SRC= $(LIBSRC)

EXHEADER= des.h des_old.h
HEADER=	des_locl.h rpc_des.h spr.h des_ver.h $(EXHEADER)

ALL=    $(GENERAL) $(SRC) $(HEADER)

top:
	(cd ../..; $(MAKE) DIRS=crypto SDIRS=$(DIR) sub_all)

all:	lib

lib:	$(LIBOBJ)
	$(AR) $(LIB) $(LIBOBJ)
	$(RANLIB) $(LIB) || echo Never mind.
	@touch lib

des: des.o cbc3_enc.o lib
	$(CC) $(CFLAGS) -o des des.o cbc3_enc.o $(LIB)

des_enc-sparc.S:	asm/des_enc.m4
	m4 -B 8192 asm/des_enc.m4 > des_enc-sparc.S
dest4-sparcv9.s:	asm/dest4-sparcv9.pl
	$(PERL) asm/dest4-sparcv9.pl $(CFLAGS) > $@

des-586.s:	asm/des-586.pl ../perlasm/x86asm.pl ../perlasm/cbc.pl
	$(PERL) asm/des-586.pl $(PERLASM_SCHEME) $(CFLAGS) > $@
crypt586.s:	asm/crypt586.pl ../perlasm/x86asm.pl ../perlasm/cbc.pl
	$(PERL) asm/crypt586.pl $(PERLASM_SCHEME) $(CFLAGS) > $@

files:
	$(PERL) $(TOP)/util/files.pl Makefile >> $(TOP)/MINFO

links:
	@$(PERL) $(TOP)/util/mklink.pl ../../include/openssl $(EXHEADER)
	@$(PERL) $(TOP)/util/mklink.pl ../../test $(TEST)
	@$(PERL) $(TOP)/util/mklink.pl ../../apps $(APPS)

# We need to use force because 'install' matches 'INSTALL' on case
# insensitive systems
FRC.install:
install: FRC.install
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
	rm -f *.s *.o *.obj des lib tags core .pure .nfs* *.old *.bak fluff

# DO NOT DELETE THIS LINE -- make depend depends on it.

cbc_cksm.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
cbc_cksm.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
cbc_cksm.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
cbc_cksm.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
cbc_cksm.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
cbc_cksm.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
cbc_cksm.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
cbc_cksm.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
cbc_cksm.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
cbc_cksm.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
cbc_cksm.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h cbc_cksm.c
cbc_cksm.o: des_locl.h
cbc_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
cbc_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
cbc_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
cbc_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
cbc_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
cbc_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
cbc_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
cbc_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
cbc_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
cbc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
cbc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h cbc_enc.c des_locl.h
cbc_enc.o: ncbc_enc.c
cfb64ede.o: ../../e_os.h ../../include/openssl/des.h
cfb64ede.o: ../../include/openssl/des_old.h ../../include/openssl/e_os2.h
cfb64ede.o: ../../include/openssl/opensslconf.h
cfb64ede.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
cfb64ede.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
cfb64ede.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
cfb64ede.o: c:\mingw\include\_mingw.h c:\mingw\include\basetsd.h
cfb64ede.o: c:\mingw\include\errno.h c:\mingw\include\fcntl.h
cfb64ede.o: c:\mingw\include\io.h c:\mingw\include\malloc.h
cfb64ede.o: c:\mingw\include\poppack.h c:\mingw\include\pshpack4.h
cfb64ede.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
cfb64ede.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
cfb64ede.o: c:\mingw\include\time.h c:\mingw\include\winbase.h
cfb64ede.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
cfb64ede.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
cfb64ede.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
cfb64ede.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
cfb64ede.o: c:\mingw\include\winreg.h c:\mingw\include\winsock2.h
cfb64ede.o: c:\mingw\include\winsvc.h c:\mingw\include\winuser.h
cfb64ede.o: c:\mingw\include\winver.h c:\mingw\include\ws2tcpip.h
cfb64ede.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
cfb64ede.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h cfb64ede.c
cfb64ede.o: des_locl.h
cfb64enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
cfb64enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
cfb64enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
cfb64enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
cfb64enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
cfb64enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
cfb64enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
cfb64enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
cfb64enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
cfb64enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
cfb64enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h cfb64enc.c
cfb64enc.o: des_locl.h
cfb_enc.o: ../../e_os.h ../../include/openssl/des.h
cfb_enc.o: ../../include/openssl/des_old.h ../../include/openssl/e_os2.h
cfb_enc.o: ../../include/openssl/opensslconf.h ../../include/openssl/ossl_typ.h
cfb_enc.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
cfb_enc.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
cfb_enc.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
cfb_enc.o: c:\mingw\include\assert.h c:\mingw\include\basetsd.h
cfb_enc.o: c:\mingw\include\errno.h c:\mingw\include\fcntl.h
cfb_enc.o: c:\mingw\include\io.h c:\mingw\include\malloc.h
cfb_enc.o: c:\mingw\include\poppack.h c:\mingw\include\pshpack4.h
cfb_enc.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
cfb_enc.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
cfb_enc.o: c:\mingw\include\time.h c:\mingw\include\winbase.h
cfb_enc.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
cfb_enc.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
cfb_enc.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
cfb_enc.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
cfb_enc.o: c:\mingw\include\winreg.h c:\mingw\include\winsock2.h
cfb_enc.o: c:\mingw\include\winsvc.h c:\mingw\include\winuser.h
cfb_enc.o: c:\mingw\include\winver.h c:\mingw\include\ws2tcpip.h
cfb_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
cfb_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h cfb_enc.c des_locl.h
des_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
des_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
des_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
des_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
des_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
des_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
des_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
des_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
des_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
des_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
des_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_enc.c des_locl.h
des_enc.o: ncbc_enc.c spr.h
des_old.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
des_old.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
des_old.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
des_old.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
des_old.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
des_old.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
des_old.o: c:\mingw\include\basetsd.h c:\mingw\include\poppack.h
des_old.o: c:\mingw\include\pshpack4.h c:\mingw\include\stdlib.h
des_old.o: c:\mingw\include\string.h c:\mingw\include\winbase.h
des_old.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
des_old.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
des_old.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
des_old.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
des_old.o: c:\mingw\include\winreg.h c:\mingw\include\winsvc.h
des_old.o: c:\mingw\include\winuser.h c:\mingw\include\winver.h
des_old.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
des_old.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_old.c
des_old2.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
des_old2.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
des_old2.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
des_old2.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
des_old2.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
des_old2.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
des_old2.o: c:\mingw\include\basetsd.h c:\mingw\include\poppack.h
des_old2.o: c:\mingw\include\pshpack4.h c:\mingw\include\stdlib.h
des_old2.o: c:\mingw\include\string.h c:\mingw\include\winbase.h
des_old2.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
des_old2.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
des_old2.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
des_old2.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
des_old2.o: c:\mingw\include\winreg.h c:\mingw\include\winsvc.h
des_old2.o: c:\mingw\include\winuser.h c:\mingw\include\winver.h
des_old2.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
des_old2.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_old2.c
ecb3_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
ecb3_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
ecb3_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
ecb3_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
ecb3_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
ecb3_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
ecb3_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
ecb3_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
ecb3_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
ecb3_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
ecb3_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
ecb3_enc.o: ecb3_enc.c
ecb_enc.o: ../../include/openssl/bio.h ../../include/openssl/crypto.h
ecb_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
ecb_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
ecb_enc.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
ecb_enc.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
ecb_enc.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
ecb_enc.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
ecb_enc.o: c:\mingw\include\errno.h c:\mingw\include\io.h
ecb_enc.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
ecb_enc.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
ecb_enc.o: c:\mingw\include\time.h
ecb_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
ecb_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h des_ver.h
ecb_enc.o: ecb_enc.c
ede_cbcm_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
ede_cbcm_enc.o: ../../include/openssl/e_os2.h
ede_cbcm_enc.o: ../../include/openssl/opensslconf.h
ede_cbcm_enc.o: ../../include/openssl/ossl_typ.h
ede_cbcm_enc.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
ede_cbcm_enc.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
ede_cbcm_enc.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
ede_cbcm_enc.o: c:\mingw\include\errno.h c:\mingw\include\io.h
ede_cbcm_enc.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
ede_cbcm_enc.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
ede_cbcm_enc.o: c:\mingw\include\time.h
ede_cbcm_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
ede_cbcm_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
ede_cbcm_enc.o: ede_cbcm_enc.c
enc_read.o: ../../e_os.h ../../include/openssl/bio.h
enc_read.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
enc_read.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
enc_read.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
enc_read.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
enc_read.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
enc_read.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
enc_read.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
enc_read.o: ../../include/openssl/ui_compat.h ../../ms/applink.c
enc_read.o: ../../ms/uplink.h ../cryptlib.h c:\mingw\include\_mingw.h
enc_read.o: c:\mingw\include\basetsd.h c:\mingw\include\errno.h
enc_read.o: c:\mingw\include\fcntl.h c:\mingw\include\io.h
enc_read.o: c:\mingw\include\malloc.h c:\mingw\include\poppack.h
enc_read.o: c:\mingw\include\pshpack4.h c:\mingw\include\stdio.h
enc_read.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
enc_read.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
enc_read.o: c:\mingw\include\winbase.h c:\mingw\include\wincon.h
enc_read.o: c:\mingw\include\windef.h c:\mingw\include\windows.h
enc_read.o: c:\mingw\include\winerror.h c:\mingw\include\wingdi.h
enc_read.o: c:\mingw\include\winnetwk.h c:\mingw\include\winnls.h
enc_read.o: c:\mingw\include\winnt.h c:\mingw\include\winreg.h
enc_read.o: c:\mingw\include\winsock2.h c:\mingw\include\winsvc.h
enc_read.o: c:\mingw\include\winuser.h c:\mingw\include\winver.h
enc_read.o: c:\mingw\include\ws2tcpip.h
enc_read.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
enc_read.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
enc_read.o: enc_read.c
enc_writ.o: ../../e_os.h ../../include/openssl/bio.h
enc_writ.o: ../../include/openssl/buffer.h ../../include/openssl/crypto.h
enc_writ.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
enc_writ.o: ../../include/openssl/e_os2.h ../../include/openssl/err.h
enc_writ.o: ../../include/openssl/lhash.h ../../include/openssl/opensslconf.h
enc_writ.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
enc_writ.o: ../../include/openssl/rand.h ../../include/openssl/safestack.h
enc_writ.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
enc_writ.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
enc_writ.o: ../../ms/applink.c ../../ms/uplink.h ../cryptlib.h
enc_writ.o: c:\mingw\include\_mingw.h c:\mingw\include\basetsd.h
enc_writ.o: c:\mingw\include\errno.h c:\mingw\include\fcntl.h
enc_writ.o: c:\mingw\include\io.h c:\mingw\include\malloc.h
enc_writ.o: c:\mingw\include\poppack.h c:\mingw\include\pshpack4.h
enc_writ.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
enc_writ.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
enc_writ.o: c:\mingw\include\time.h c:\mingw\include\winbase.h
enc_writ.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
enc_writ.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
enc_writ.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
enc_writ.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
enc_writ.o: c:\mingw\include\winreg.h c:\mingw\include\winsock2.h
enc_writ.o: c:\mingw\include\winsvc.h c:\mingw\include\winuser.h
enc_writ.o: c:\mingw\include\winver.h c:\mingw\include\ws2tcpip.h
enc_writ.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
enc_writ.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
enc_writ.o: enc_writ.c
fcrypt.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
fcrypt.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
fcrypt.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
fcrypt.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
fcrypt.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
fcrypt.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
fcrypt.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
fcrypt.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
fcrypt.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
fcrypt.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
fcrypt.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h fcrypt.c
fcrypt_b.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
fcrypt_b.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
fcrypt_b.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
fcrypt_b.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
fcrypt_b.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
fcrypt_b.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
fcrypt_b.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
fcrypt_b.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
fcrypt_b.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
fcrypt_b.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
fcrypt_b.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
fcrypt_b.o: fcrypt_b.c
ofb64ede.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
ofb64ede.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
ofb64ede.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
ofb64ede.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
ofb64ede.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
ofb64ede.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
ofb64ede.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
ofb64ede.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
ofb64ede.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
ofb64ede.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
ofb64ede.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
ofb64ede.o: ofb64ede.c
ofb64enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
ofb64enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
ofb64enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
ofb64enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
ofb64enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
ofb64enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
ofb64enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
ofb64enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
ofb64enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
ofb64enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
ofb64enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
ofb64enc.o: ofb64enc.c
ofb_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
ofb_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
ofb_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
ofb_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
ofb_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
ofb_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
ofb_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
ofb_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
ofb_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
ofb_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
ofb_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h ofb_enc.c
pcbc_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
pcbc_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
pcbc_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
pcbc_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
pcbc_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
pcbc_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
pcbc_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
pcbc_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
pcbc_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
pcbc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
pcbc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
pcbc_enc.o: pcbc_enc.c
qud_cksm.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
qud_cksm.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
qud_cksm.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
qud_cksm.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
qud_cksm.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
qud_cksm.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
qud_cksm.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
qud_cksm.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
qud_cksm.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
qud_cksm.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
qud_cksm.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
qud_cksm.o: qud_cksm.c
rand_key.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
rand_key.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
rand_key.o: ../../include/openssl/ossl_typ.h ../../include/openssl/rand.h
rand_key.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
rand_key.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
rand_key.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
rand_key.o: c:\mingw\include\basetsd.h c:\mingw\include\poppack.h
rand_key.o: c:\mingw\include\pshpack4.h c:\mingw\include\stdlib.h
rand_key.o: c:\mingw\include\string.h c:\mingw\include\winbase.h
rand_key.o: c:\mingw\include\wincon.h c:\mingw\include\windef.h
rand_key.o: c:\mingw\include\windows.h c:\mingw\include\winerror.h
rand_key.o: c:\mingw\include\wingdi.h c:\mingw\include\winnetwk.h
rand_key.o: c:\mingw\include\winnls.h c:\mingw\include\winnt.h
rand_key.o: c:\mingw\include\winreg.h c:\mingw\include\winsvc.h
rand_key.o: c:\mingw\include\winuser.h c:\mingw\include\winver.h
rand_key.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
rand_key.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h rand_key.c
read2pwd.o: ../../include/openssl/crypto.h ../../include/openssl/des.h
read2pwd.o: ../../include/openssl/des_old.h ../../include/openssl/e_os2.h
read2pwd.o: ../../include/openssl/opensslconf.h
read2pwd.o: ../../include/openssl/opensslv.h ../../include/openssl/ossl_typ.h
read2pwd.o: ../../include/openssl/safestack.h ../../include/openssl/stack.h
read2pwd.o: ../../include/openssl/symhacks.h ../../include/openssl/ui.h
read2pwd.o: ../../include/openssl/ui_compat.h c:\mingw\include\_mingw.h
read2pwd.o: c:\mingw\include\stdio.h c:\mingw\include\stdlib.h
read2pwd.o: c:\mingw\include\string.h c:\mingw\include\sys\types.h
read2pwd.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
read2pwd.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h read2pwd.c
rpc_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
rpc_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
rpc_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
rpc_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
rpc_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
rpc_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
rpc_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
rpc_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
rpc_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
rpc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
rpc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h des_ver.h
rpc_enc.o: rpc_des.h rpc_enc.c
set_key.o: ../../include/openssl/crypto.h ../../include/openssl/des.h
set_key.o: ../../include/openssl/des_old.h ../../include/openssl/e_os2.h
set_key.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
set_key.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
set_key.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
set_key.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
set_key.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
set_key.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
set_key.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
set_key.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
set_key.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
set_key.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h set_key.c
str2key.o: ../../include/openssl/crypto.h ../../include/openssl/des.h
str2key.o: ../../include/openssl/des_old.h ../../include/openssl/e_os2.h
str2key.o: ../../include/openssl/opensslconf.h ../../include/openssl/opensslv.h
str2key.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
str2key.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
str2key.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
str2key.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
str2key.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
str2key.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
str2key.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
str2key.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
str2key.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h str2key.c
xcbc_enc.o: ../../include/openssl/des.h ../../include/openssl/des_old.h
xcbc_enc.o: ../../include/openssl/e_os2.h ../../include/openssl/opensslconf.h
xcbc_enc.o: ../../include/openssl/ossl_typ.h ../../include/openssl/safestack.h
xcbc_enc.o: ../../include/openssl/stack.h ../../include/openssl/symhacks.h
xcbc_enc.o: ../../include/openssl/ui.h ../../include/openssl/ui_compat.h
xcbc_enc.o: c:\mingw\include\_mingw.h c:\mingw\include\errno.h
xcbc_enc.o: c:\mingw\include\io.h c:\mingw\include\stdio.h
xcbc_enc.o: c:\mingw\include\stdlib.h c:\mingw\include\string.h
xcbc_enc.o: c:\mingw\include\sys\types.h c:\mingw\include\time.h
xcbc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stdarg.h
xcbc_enc.o: c:\mingw\lib\gcc\mingw32\4.8.1\include\stddef.h des_locl.h
xcbc_enc.o: xcbc_enc.c
