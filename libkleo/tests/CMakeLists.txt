kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(test_keylister_SRCS test_keylister.cpp )

kde4_automoc(${test_keylister_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_keylister ${test_keylister_SRCS})

target_link_libraries(test_keylister  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_keygen_SRCS test_keygen.cpp )

kde4_automoc(${test_keygen_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_keygen ${test_keygen_SRCS})

target_link_libraries(test_keygen  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_keyselectiondialog_SRCS test_keyselectiondialog.cpp )

kde4_automoc(${test_keyselectiondialog_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_keyselectiondialog ${test_keyselectiondialog_SRCS})

target_link_libraries(test_keyselectiondialog  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_cryptoconfig_SRCS test_cryptoconfig.cpp )

kde4_automoc(${test_cryptoconfig_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_cryptoconfig ${test_cryptoconfig_SRCS})

target_link_libraries(test_cryptoconfig  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_gnupgprocessbase_SRCS test_gnupgprocessbase.cpp )

kde4_automoc(${test_gnupgprocessbase_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_gnupgprocessbase ${test_gnupgprocessbase_SRCS})

target_link_libraries(test_gnupgprocessbase  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_jobs_SRCS test_jobs.cpp )

kde4_automoc(${test_jobs_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(test_jobs ${test_jobs_SRCS})

target_link_libraries(test_jobs  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libkleo \
#	 \
#	$(GPGME_CFLAGS) $(all_includes)
#
#check_PROGRAMS = test_keylister test_keygen test_keyselectiondialog \
#		test_cryptoconfig test_gnupgprocessbase test_jobs
#
#test_keylister_SOURCES = test_keylister.cpp
#test_keygen_SOURCES = test_keygen.cpp
#test_keyselectiondialog_SOURCES = test_keyselectiondialog.cpp
#test_cryptoconfig_SOURCES = test_cryptoconfig.cpp
#test_gnupgprocessbase_SOURCES = test_gnupgprocessbase.cpp
#test_jobs_SOURCES = test_jobs.cpp
#
#LDADD = ../libkleopatra.la
#
#METASOURCES = AUTO
#
