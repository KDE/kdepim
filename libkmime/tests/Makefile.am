AM_CPPFLAGS = -I$(top_srcdir)/libkmime $(all_includes)
LDADD = ../libkmime.la

# test programs:
check_PROGRAMS = test_kmime_header_parsing \
		test_charfreq \
		test_mdn \
		test_dates \
		test_kmime_codec

test_kmime_codec_SOURCES = test_kmime_codec.cpp
test_charfreq_SOURCES = test_charfreq.cpp
test_mdn_SOURCES = test_mdn.cpp
test_dates_SOURCES = test_dates.cpp
test_kmime_header_parsing_SOURCES = test_kmime_header_parsing.cpp

