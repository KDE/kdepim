kde4_header()

add_subdirectory( libical ) 
add_subdirectory( versit ) 
add_subdirectory( tests ) 

include_directories( ${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/ktnef ${CMAKE_SOURCE_DIR}/libemailfunctions ${CMAKE_SOURCE_DIR}/libkmime ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcal_LIB_SRCS 
   incidencebase.cpp 
   incidence.cpp 
   journal.cpp 
   todo.cpp 
   event.cpp 
   freebusy.cpp 
   attendee.cpp 
   attachment.cpp 
   recurrencerule.cpp 
   recurrence.cpp 
   alarm.cpp 
   customproperties.cpp 
   calendar.cpp 
   calendarlocal.cpp 
   calformat.cpp 
   vcalformat.cpp 
   icalformat.cpp 
   icalformatimpl.cpp 
   incidenceformatter.cpp 
   vcaldrag.cpp 
   icaldrag.cpp 
   exceptions.cpp 
   scheduler.cpp 
   imipscheduler.cpp 
   dummyscheduler.cpp 
   calfilter.cpp 
   person.cpp 
   period.cpp 
   duration.cpp 
   dndfactory.cpp 
   filestorage.cpp 
   compat.cpp 
   resourcecalendar.cpp 
   resourcelocal.cpp 
   resourcelocalconfig.cpp 
   resourcelocaldir.cpp 
   resourcelocaldirconfig.cpp 
   resourcecached.cpp 
   resourcecachedconfig.cpp 
   calendarresources.cpp 
   qtopiaformat.cpp 
   htmlexport.cpp 
   calendarnull.cpp 
   freebusyurlstore.cpp 
   confirmsavedialog.cpp 
   icaltimezones.cpp )

kde4_automoc(${kcal_LIB_SRCS})

kde4_add_kcfg_files(kcal_LIB_SRCS htmlexportsettings.kcfgc )

kde4_add_library(kcal SHARED ${kcal_LIB_SRCS})

target_link_libraries(kcal  ${QT_AND_KDECORE_LIBS} versit kio ical icalss ktnef emailfunctions kmime kresources kabc kde3support )

set_target_properties(kcal PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install_targets(${LIB_INSTALL_DIR} kcal )


########### next target ###############

set(kcal_local_PART_SRCS resourcelocal_plugin.cpp )

kde4_automoc(${kcal_local_PART_SRCS})

kde4_add_plugin(kcal_local ${kcal_local_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_local )

target_link_libraries(kcal_local  ${QT_AND_KDECORE_LIBS} kcal )

install_targets(${LIB_INSTALL_DIR}/kde4 kcal_local )


########### next target ###############

set(kcal_localdir_PART_SRCS resourcelocaldir_plugin.cpp )

kde4_automoc(${kcal_localdir_PART_SRCS})

kde4_add_plugin(kcal_localdir ${kcal_localdir_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcal_localdir )

target_link_libraries(kcal_localdir  ${QT_AND_KDECORE_LIBS} kcal )

install_targets(${LIB_INSTALL_DIR}/kde4 kcal_localdir )


########### next target ###############

set(convertqtopia_SRCS convertqtopia.cpp )

kde4_automoc(${convertqtopia_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(convertqtopia ${convertqtopia_SRCS})

target_link_libraries(convertqtopia  ${QT_AND_KDECORE_LIBS} kcal )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kcal FILES local.desktop localdir.desktop )
install_files( /include/libkcal FILES alarm.h attachment.h attendee.h calendar.h    calendarlocal.h calendarnull.h calendarresources.h calfilter.h calformat.h    calstorage.h customproperties.h dndfactory.h duration.h event.h    exceptions.h filestorage.h freebusy.h htmlexportsettings.h htmlexport.h icaldrag.h    icalformat.h icaltimezones.h incidencebase.h incidence.h incidenceformatter.h    journal.h kcalversion.h listbase.h period.h person.h    qtopiaformat.h recurrencerule.h recurrence.h resourcecached.h resourcecalendar.h    resourcelocalconfig.h resourcelocaldirconfig.h resourcelocaldir.h    resourcelocal.h scheduler.h libkcal_export.h    todo.h vcaldrag.h vcalformat.h )
install_files( ${SERVICES_INSTALL_DIR}/kresources FILES kcal_manager.desktop )

kde4_create_doxygen_docs( REFERENCES kresources EXCLUDE libical  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = libical versit tests
#
#INCLUDES = -I$(srcdir)/libical/src/libical \
#  -I$(srcdir)/libical/src/libicalss -I./libical/src/libical \
#  -I./libical/src/libicalss -I$(srcdir)/versit -I$(top_srcdir)/ \
#  -I$(top_srcdir)/ktnef \
#  -I$(top_srcdir)/libemailfunctions \
#  -I$(top_srcdir)/libkmime \
#  $(all_includes)
#
#lib_LTLIBRARIES   = libkcal.la
#libkcal_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 2:0:0
#libkcal_la_LIBADD  = versit/libversit.la $(LIB_KIO) \
#                     ./libical/src/libical/libical.la \
#                     ./libical/src/libicalss/libicalss.la \
#                     $(top_builddir)/ktnef/lib/libktnef.la \
#                     ../libemailfunctions/libemailfunctions.la \
#                     $(top_builddir)/libkmime/libkmime.la \
#                     -lkresources -lkabc $(LIB_KDE3SUPPORT)
#
#libkcal_la_COMPILE_FIRST = ./libical/src/libical/ical.h ./libical/src/libicalss/icalss.h
#libkcal_la_SOURCES = \
#	incidencebase.cpp incidence.cpp journal.cpp todo.cpp event.cpp \
#	freebusy.cpp attendee.cpp attachment.cpp recurrencerule.cpp recurrence.cpp alarm.cpp \
#	customproperties.cpp calendar.cpp calendarlocal.cpp \
#	calformat.cpp vcalformat.cpp icalformat.cpp icalformatimpl.cpp \
#	incidenceformatter.cpp \
#	vcaldrag.cpp icaldrag.cpp \
#	exceptions.cpp \
#	scheduler.cpp imipscheduler.cpp dummyscheduler.cpp \
#	calfilter.cpp \
#	person.cpp period.cpp duration.cpp \
#	dndfactory.cpp \
#	filestorage.cpp \
#	compat.cpp \
#	resourcecalendar.cpp resourcelocal.cpp resourcelocalconfig.cpp \
#	resourcelocaldir.cpp resourcelocaldirconfig.cpp \
#	resourcecached.cpp resourcecachedconfig.cpp \
#	calendarresources.cpp \
#	qtopiaformat.cpp \
#	htmlexportsettings.kcfgc htmlexport.cpp calendarnull.cpp \
#	freebusyurlstore.cpp \
#	confirmsavedialog.cpp icaltimezones.cpp
#
#libkcalincludedir = $(includedir)/libkcal
#libkcalinclude_HEADERS = alarm.h attachment.h attendee.h calendar.h \
#  calendarlocal.h calendarnull.h calendarresources.h calfilter.h calformat.h \
#  calstorage.h customproperties.h dndfactory.h duration.h event.h \
#  exceptions.h filestorage.h freebusy.h htmlexportsettings.h htmlexport.h icaldrag.h \
#  icalformat.h icaltimezones.h incidencebase.h incidence.h incidenceformatter.h \
#  journal.h kcalversion.h listbase.h period.h person.h \
#  qtopiaformat.h recurrencerule.h recurrence.h resourcecached.h resourcecalendar.h \
#  resourcelocalconfig.h resourcelocaldirconfig.h resourcelocaldir.h \
#  resourcelocal.h scheduler.h libkcal_export.h \
#  todo.h vcaldrag.h vcalformat.h
#
#kde_module_LTLIBRARIES = kcal_local.la kcal_localdir.la 
#
#kcal_local_la_SOURCES = resourcelocal_plugin.cpp
#kcal_local_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_local_la_LIBADD  = libkcal.la
#
#kcal_localdir_la_SOURCES = resourcelocaldir_plugin.cpp
#kcal_localdir_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kcal_localdir_la_LIBADD  = libkcal.la
#
#EXTRA_PROGRAMS = convertqtopia
#convertqtopia_SOURCES = convertqtopia.cpp
#convertqtopia_LDADD = libkcal.la
#
#servicedir = $(kde_servicesdir)/kresources/kcal
#service_DATA = local.desktop localdir.desktop
#
#manager_DATA = kcal_manager.desktop
#managerdir = $(kde_servicesdir)/kresources
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/libkcal.pot
#
#DOXYGEN_EXCLUDE = libical
#DOXYGEN_REFERENCES=kresources
#include $(top_srcdir)/admin/Doxyfile.am
