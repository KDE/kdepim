#KDE_CXXFLAGS = 
#SUBDIRS = 
AM_CPPFLAGS = -I$(top_srcdir)/libkdenetwork $(all_includes)
LDADD = ../libkdenetwork.la

# test programs:
check_PROGRAMS = test_kmime_header_parsing \
		test_charfreq \
		test_mdn \
		test_dates \
		test_kmime_codec \
		testutf7decoder \
		testutf7encoder \
		test_kregexp

test_kmime_codec_SOURCES = test_kmime_codec.cpp
test_charfreq_SOURCES = test_charfreq.cpp
test_mdn_SOURCES = test_mdn.cpp
test_dates_SOURCES = test_dates.cpp
test_kmime_header_parsing_SOURCES = test_kmime_header_parsing.cpp
testutf7encoder_SOURCES = testutf7encoder2.cpp
testutf7decoder_SOURCES = testutf7decoder.cpp
test_kregexp_SOURCES = test_kregexp.cpp

