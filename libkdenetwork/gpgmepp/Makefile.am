KDE_OPTIONS = foreign
#AM_CXXFLAGS = -Wno-deprecated-declarations

SUBDIRS = interfaces .

INCLUDES = -I$(top_srcdir)/libkdenetwork -I$(top_srcdir)/libkdenetwork/gpgmepp \
	$(GPGME_CFLAGS) -I$(top_builddir)/libkdepim $(all_includes)

gpgmeppdir = $(includedir)/gpgme++
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
		encryptionresult.h \
		engineinfo.h

noinst_HEADERS = context_p.h data_p.h shared.h callbacks.h gpgme-0-3-compat.h result_p.h util.h

lib_LTLIBRARIES = libgpgme++.la
libgpgme___la_SOURCES = context.cpp \
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
			encryptionresult.cpp \
			engineinfo.cpp

# --version-info CURRENT:REVISION:AGE
#   (Code changed:                      REVISION++)
#   (Interfaces added/removed/changed:  CURRENT++, REVISION=0)
#   (Interfaces added:                  AGE++)
#   (Interfaces removed/changed:        AGE=0)
libgpgme___la_LDFLAGS = -no-undefined -version-info 1:0:1
libgpgme___la_LIBADD = $(GPGME_LIBS)
libgpgme___la_DEPENDENCIES = $(GPGME_LIBS_DEP)

