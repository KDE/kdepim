KDE_CXXFLAGS = $(USE_RTTI)
INCLUDES= $(all_includes)

if needs_gpgme_copy
GPGME_COPY_SUBDIRS = libgpg-error-copy libgpgme-copy
endif

SUBDIRS = $(GPGME_COPY_SUBDIRS) gpgmepp qgpgme . pics4kmail

lib_LTLIBRARIES = libkdenetwork.la

libkdenetwork_la_SOURCES = kscoring.cpp \
                          kscoringeditor.cpp \
			  kregexp3.cpp \
			  qutf7codec.cpp \
			  \
			  kpgpblock.cpp \
                          kpgpui.cpp \
                          kpgpkey.cpp \
                          kpgp.cpp \
                          kpgpbase.cpp \
                          kpgpbaseG.cpp \
                          kpgpbase2.cpp \
                          kpgpbase5.cpp \
                          kpgpbase6.cpp \
			  \
                          kwidgetlister.cpp \
			  kqcstringsplitter.cpp \
			  boolflags.cpp	\
			  \
			  kmime_charfreq.cpp \
			  kmime_util.cpp \
			  kmime_mdn.cpp \
			  kmime_codecs.cpp \
			  kmime_codec_base64.cpp \
			  kmime_codec_uuencode.cpp \
			  kmime_codec_qp.cpp \
			  kmime_codec_identity.cpp \
			  kmime_parsers.cpp \
			  kmime_header_parsing.cpp \
			  kmime_content.cpp \
			  kmime_headers.cpp \
			  kmime_message.cpp \
			  kmime_newsarticle.cpp \
			  \
			  kfoldertree.cpp \
			  ksubscription.cpp \
			  kaccount.cpp \
			  spellingfilter.cpp \
			  linklocator.cpp \
			  recentaddresses.cpp

libkdenetwork_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 4:0:2
noinst_HEADERS = kpgpbase.h

libkdenetwork_la_LIBADD = $(LIB_KDEUI) $(LIB_KSPELL) $(LIB_KABC) $(LIB_POLL)

METASOURCES = AUTO

updatedir = $(kde_datadir)/kconf_update
update_DATA = kpgp.upd
update_SCRIPTS = kpgp-3.1-upgrade-address-data.pl

messages: 
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkdenetwork.pot

include ../admin/Doxyfile.am
