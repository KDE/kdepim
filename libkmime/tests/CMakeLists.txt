kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(test_kmime_header_parsing_SRCS test_kmime_header_parsing.cpp )

kde4_automoc(${test_kmime_header_parsing_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_kmime_header_parsing ${test_kmime_header_parsing_SRCS})

target_link_libraries(test_kmime_header_parsing  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_charfreq_SRCS test_charfreq.cpp )

kde4_automoc(${test_charfreq_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_charfreq ${test_charfreq_SRCS})

target_link_libraries(test_charfreq  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_mdn_SRCS test_mdn.cpp )

kde4_automoc(${test_mdn_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_mdn ${test_mdn_SRCS})

target_link_libraries(test_mdn  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_dates_SRCS test_dates.cpp )

kde4_automoc(${test_dates_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_dates ${test_dates_SRCS})

target_link_libraries(test_dates  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_kmime_codec_SRCS test_kmime_codec.cpp )

kde4_automoc(${test_kmime_codec_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_kmime_codec ${test_kmime_codec_SRCS})

target_link_libraries(test_kmime_codec  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(rfc2047test_SRCS rfc2047test.cpp )

kde4_automoc(${rfc2047test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(rfc2047test ${rfc2047test_SRCS})

target_link_libraries(rfc2047test  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmime_util_test_SRCS kmime_util_test.cpp )

kde4_automoc(${kmime_util_test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(kmime_util_test ${kmime_util_test_SRCS})

target_link_libraries(kmime_util_test  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -I$(top_srcdir)/libkmime $(all_includes)
#LDADD = ../libkmime.la -lQtTest_debug
#
#METASOURCES = AUTO
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
## test programs:
#check_PROGRAMS = test_kmime_header_parsing \
#		test_charfreq \
#		test_mdn \
#		test_dates \
#		test_kmime_codec \
#		rfc2047test \
#		kmime_util_test
#
#test_kmime_codec_SOURCES = test_kmime_codec.cpp
#test_charfreq_SOURCES = test_charfreq.cpp
#test_mdn_SOURCES = test_mdn.cpp
#test_dates_SOURCES = test_dates.cpp
#test_kmime_header_parsing_SOURCES = test_kmime_header_parsing.cpp
#rfc2047test_SOURCES = rfc2047test.cpp
#kmime_util_test_SOURCES = kmime_util_test.cpp
#
#TESTS = rfc2047test kmime_util_test
#
