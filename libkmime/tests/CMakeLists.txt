
if(KDE4_BUILD_TESTS)

include_directories(
	${CMAKE_SOURCE_DIR}/libkmime
	
)


########### next target ###############

set(test_kmime_header_parsing_SRCS test_kmime_header_parsing.cpp )

kde4_automoc(${test_kmime_header_parsing_SRCS})

kde4_add_executable(test_kmime_header_parsing ${test_kmime_header_parsing_SRCS})

target_link_libraries(test_kmime_header_parsing  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_charfreq_SRCS test_charfreq.cpp )

kde4_automoc(${test_charfreq_SRCS})

kde4_add_executable(test_charfreq ${test_charfreq_SRCS})

target_link_libraries(test_charfreq  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_mdn_SRCS test_mdn.cpp )

kde4_automoc(${test_mdn_SRCS})

kde4_add_executable(test_mdn ${test_mdn_SRCS})

target_link_libraries(test_mdn  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_dates_SRCS test_dates.cpp )

kde4_automoc(${test_dates_SRCS})

kde4_add_executable(test_dates ${test_dates_SRCS})

target_link_libraries(test_dates  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_kmime_codec_SRCS test_kmime_codec.cpp )

kde4_automoc(${test_kmime_codec_SRCS})

kde4_add_executable(test_kmime_codec ${test_kmime_codec_SRCS})

target_link_libraries(test_kmime_codec  kmime ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(rfc2047test_SRCS rfc2047test.cpp )

kde4_automoc(${rfc2047test_SRCS})

kde4_add_executable(rfc2047test RUN_UNINSTALLED ${rfc2047test_SRCS})

target_link_libraries(rfc2047test kmime ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(libkmime-rfc2047test ${EXECUTABLE_OUTPUT_PATH}/rfc2047test)

########### next target ###############

set(kmime_util_test_SRCS kmime_util_test.cpp )

kde4_automoc(${kmime_util_test_SRCS})

kde4_add_executable(kmime_util_test RUN_UNINSTALLED ${kmime_util_test_SRCS})

target_link_libraries(kmime_util_test  kmime ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(libkmime-kmime_util_test ${EXECUTABLE_OUPUT_PATH}/kmime_util_test)

########### install files ###############

endif(KDE4_BUILD_TESTS)




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
