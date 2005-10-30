INCLUDES= -I$(top_srcdir)/libemailfunctions \
        $(all_includes)

lib_LTLIBRARIES = libkmime.la

libkmime_la_SOURCES = \
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
			  boolflags.cpp \
			  kqcstringsplitter.cpp \
			  kautodeletehash.cpp

libkmime_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 4:0:2

libkmime_la_LIBADD = $(top_builddir)/libemailfunctions/libemailfunctions.la \
                     $(LIB_KDECORE) $(LIB_POLL)

METASOURCES = AUTO

messages: 
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkmime.pot

DOXYGEN_EXCLUDE =  kmime_headers_obs.h
include $(top_srcdir)/admin/Doxyfile.am
