kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkcal ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(loadcalendar_SRCS loadcalendar.cpp )

kde4_automoc(${loadcalendar_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(loadcalendar ${loadcalendar_SRCS})

target_link_libraries(loadcalendar  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testtostring_SRCS testtostring.cpp )

kde4_automoc(${testtostring_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testtostring ${testtostring_SRCS})

target_link_libraries(testtostring  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testincidence_SRCS testincidence.cpp )

kde4_automoc(${testincidence_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testincidence ${testincidence_SRCS})

target_link_libraries(testincidence  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testcalendar_SRCS testcalendar.cpp )

kde4_automoc(${testcalendar_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testcalendar ${testcalendar_SRCS})

target_link_libraries(testcalendar  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(readandwrite_SRCS readandwrite.cpp )

kde4_automoc(${readandwrite_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(readandwrite ${readandwrite_SRCS})

target_link_libraries(readandwrite  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testresource_SRCS testresource.cpp )

kde4_automoc(${testresource_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testresource ${testresource_SRCS})

target_link_libraries(testresource  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testfields_SRCS testfields.cpp )

kde4_automoc(${testfields_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testfields ${testfields_SRCS})

target_link_libraries(testfields  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testrecurrence_SRCS testrecurrence.cpp )

kde4_automoc(${testrecurrence_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testrecurrence ${testrecurrence_SRCS})

target_link_libraries(testrecurrence  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testrecurprevious_SRCS testrecurprevious.cpp )

kde4_automoc(${testrecurprevious_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testrecurprevious ${testrecurprevious_SRCS})

target_link_libraries(testrecurprevious  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testrecurson_SRCS testrecurson.cpp )

kde4_automoc(${testrecurson_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testrecurson ${testrecurson_SRCS})

target_link_libraries(testrecurson  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testrecurrencetype_SRCS testrecurrencetype.cpp )

kde4_automoc(${testrecurrencetype_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testrecurrencetype ${testrecurrencetype_SRCS})

target_link_libraries(testrecurrencetype  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testicaltimezones_SRCS testicaltimezones.cpp )

kde4_automoc(${testicaltimezones_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testicaltimezones ${testicaltimezones_SRCS})

target_link_libraries(testicaltimezones  ${QT_AND_KDECORE_LIBS} kcal QtTest_debug )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testvcalexport_SRCS testvcalexport.cpp )

kde4_automoc(${testvcalexport_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testvcalexport ${testvcalexport_SRCS})

target_link_libraries(testvcalexport  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libkcal -I$(top_builddir)/libkcal/libical/src \
#	-Iqttestlib $(all_includes)
#
#
#check_PROGRAMS = loadcalendar \
#                 testtostring \
#                 testincidence \
#                 testcalendar \
#                 readandwrite \
#                 testresource \
#                 testfields \
#                 testrecurrence \
#                 testrecurprevious \
#                 testrecurson \
#                 testrecurrencetype \
#		 testicaltimezones \
#                 testvcalexport
#
#
#METASOURCES = AUTO
#
#loadcalendar_SOURCES = loadcalendar.cpp
#loadcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#loadcalendar_LDADD = ../libkcal.la
#
#testincidence_SOURCES = testincidence.cpp
#testincidence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testincidence_LDADD = ../libkcal.la
#
#testcalendar_SOURCES = testcalendar.cpp
#testcalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testcalendar_LDADD = ../libkcal.la
#
#testtostring_SOURCES = testtostring.cpp
#testtostring_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testtostring_LDADD = ../libkcal.la
#
#readandwrite_SOURCES = readandwrite.cpp
#readandwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#readandwrite_LDADD = ../libkcal.la
#
#testresource_SOURCES = testresource.cpp
#testresource_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testresource_LDADD = ../libkcal.la
#
#testrecurrence_SOURCES = testrecurrence.cpp
#testrecurrence_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurrence_LDADD = ../libkcal.la
#
#testrecurprevious_SOURCES = testrecurprevious.cpp
#testrecurprevious_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurprevious_LDADD = ../libkcal.la
#
#testrecurson_SOURCES = testrecurson.cpp
#testrecurson_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurson_LDADD = ../libkcal.la
#
#testrecurrencetype_SOURCES = testrecurrencetype.cpp
#testrecurrencetype_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testrecurrencetype_LDADD = ../libkcal.la
#
#testicaltimezones_SOURCES = testicaltimezones.cpp
#testicaltimezones_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testicaltimezones_LDADD = ../libkcal.la -lQtTest_debug
#
#testvcalexport_SOURCES = testvcalexport.cpp
#testvcalexport_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testvcalexport_LDADD = ../libkcal.la
#
#testfields_SOURCES = testfields.cpp
#testfields_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testfields_LDADD   = ../libkcal.la
#testfields_CXXFLAGS= -DINPUT='"$(srcdir)/data/test_pilot.ics"'
#
#TESTFILES = test1.ics test2.ics test3.ics test4.ics test5.ics test_Mozilla.ics
#
#check-local: readandwrite testrecurrence testrecurprevious testrecurson testvcalexport testicaltimezones
#	rm -f FAILED; \
#	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl testrecurrence "next" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl testrecurprevious "prev" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/RecurrenceRule/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl testrecurson "recurson" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/Compat/ -name "*.ics"`; do \
#	  perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
#	done; \
#	for i in `find $(srcdir)/data/vCalendar/ -name "*.ics"`; do \
#		perl $(srcdir)/runtestcase.pl testvcalexport "vcal" $$i; \
#	done;
#	for i in `find $(srcdir)/data/vCalendar/ -name "*.vcs"`; do \
#		perl $(srcdir)/runtestcase.pl readandwrite "ical" $$i; \
#	done;
#	[ ! -e FAILED ]
#
