KDE_OPTIONS = foreign
#AM_CXXFLAGS = -Wno-deprecated-declarations

SUBDIRS = interfaces .

INCLUDES = -I$(top_srcdir)/libkdenetwork -I$(top_srcdir)/libkdenetwork/gpgmepp $(GPGME_CFLAGS)

gpgmeppdir = $(includedir)/gpgmepp
gpgmepp_HEADERS = context.h \
		key.h \
		trustitem.h \
		eventloopinteractor.h \
		data.h \
		gpgmefw.h \
		result.h \
		keylistresult.h \
		keygenerationresult.h \
		importresult.h \
		decryptionresult.h \
		verificationresult.h \
		signingresult.h \
		encryptionresult.h

noinst_HEADERS = context_p.h data_p.h shared.h callbacks.h gpgme-0-3-compat.h

lib_LTLIBRARIES = libgpgmepp.la
libgpgmepp_la_SOURCES = context.cpp \
			key.cpp \
			trustitem.cpp \
			data.cpp \
			callbacks.cpp \
			eventloopinteractor.cpp \
			keylistresult.cpp \
			keygenerationresult.cpp \
			importresult.cpp \
			decryptionresult.cpp \
			verificationresult.cpp \
			signingresult.cpp \
			encryptionresult.cpp

libgpgmepp_la_LDFLAGS = -no-undefined -version-info 0:0:0
libgpgmepp_la_LIBADD = $(GPGME_LIBS)

include ../../admin/Doxyfile.am
