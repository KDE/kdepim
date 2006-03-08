kde4_header()

add_subdirectory( libical ) 
add_subdirectory( versit ) 
add_subdirectory( tests ) 

ADD_DEFINITIONS(-DPACKAGE_DATA_DIR=\\""${CMAKE_INSTALL_PREFIX}${DATA_INSTALL_DIR}/libical"\\" ) # added manually

include_directories( ${CMAKE_SOURCE_DIR}/libkcal/versit/  ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/ ${CMAKE_SOURCE_DIR}/ ${CMAKE_SOURCE_DIR}/ktnef ${CMAKE_SOURCE_DIR}/libemailfunctions ${CMAKE_SOURCE_DIR}/libkmime ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

set(ICALSCRIPTS ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ )

set(PROPERTYDEPS 
	${ICALSCRIPTS}/mkderivedproperties.pl 
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/properties.csv	
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h.in 
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c.in 
)

add_custom_command(OUTPUT
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS}
${ICALSCRIPTS}/mkderivedproperties.pl -i
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h.in
-h  ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/properties.csv ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h   DEPENDS  ${PROPERTYDEPS} )


add_custom_command(OUTPUT
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c
COMMAND ${PERL_EXECUTABLE} ARGS -I${ICALSCRIPTS}
${ICALSCRIPTS}/mkderivedproperties.pl -i
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c.in
-c ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/properties.csv
${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c   DEPENDS  ${PROPERTYDEPS} ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h )


macro_additional_clean_files( 
                              ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c 
                              ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h 
)


set(PARAMETERDEPS 
	${ICALSCRIPTS}/mkderivedparameters.pl 
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv 
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c.in
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h.in
)

add_custom_command(OUTPUT
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedparameters.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h.in -h ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h   DEPENDS  ${PARAMETERDEPS} )

add_custom_command(OUTPUT
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedparameters.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c.in -c ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c   DEPENDS  ${PARAMETERDEPS} ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h )

macro_additional_clean_files( 
	{CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c 
	{CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h 
)


set(RESTRICTIONDEPS 
	${ICALSCRIPTS}/mkrestrictiontable.pl
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/restrictions.csv
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c.in
)

add_custom_command(OUTPUT
${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c
COMMAND ${PERL_EXECUTABLE} ARGS -I ${ICALSCRIPTS} ${ICALSCRIPTS}/mkrestrictiontable.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c.in ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/restrictions.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c   DEPENDS  ${RESTRICTIONDEPS} )


macro_additional_clean_files( ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c)

set(VALUEDEPS
	${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedvalues.pl
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c.in
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h.in
)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h COMMAND ${PERL_EXECUTABLE} ARGS -I${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedvalues.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h.in -h ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h   DEPENDS  ${VALUEDEPS} )


add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c COMMAND ${PERL_EXECUTABLE} ARGS -I${ICALSCRIPTS} ${ICALSCRIPTS}/mkderivedvalues.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c.in -c ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c   DEPENDS  ${VALUEDEPS} ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h )

macro_additional_clean_files(${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c )



set(libversit_SRCS 
	${CMAKE_SOURCE_DIR}/libkcal/versit/vcc.c 
	${CMAKE_SOURCE_DIR}/libkcal/versit/vobject.c 
)

set(libical_SRCS
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/caldate.c       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalarray.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalattach.c        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c  
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrecur.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c   
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalcomponent.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalenums.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalerror.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalmemory.c        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalmime.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalparameter.c    
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalparser.c        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c   
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalproperty.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icaltime.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalduration.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalperiod.c        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icaltimezone.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icaltypes.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalvalue.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c  
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/pvl.c           
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/sspm.c          
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/vsnprintf.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icallangbind.c
)

set(libicalss_SRCS
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalcalendar.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalcalendar.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalclassify.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalclassify.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalcluster.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalcluster.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalclusterimpl.h   
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalgauge.c     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalgauge.h     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalgaugeimpl.h     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icaldirset.c        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icaldirset.h        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icaldirsetimpl.h    
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalfileset.c       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalfileset.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalfilesetimpl.h   
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalset.c       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalset.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalssyacc.h        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalspanlist.c      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalspanlist.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalmessage.c       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalmessage.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalsslexer.c           
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalss/icalssyacc.c
   )

## ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering. It
## is required to make the combined header ical.h properly
set(COMBINEDHEADERS 
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalversion.h         
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icaltime.h        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalduration.h    
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalperiod.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalenums.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icaltypes.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrecur.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalattach.h     
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalicalderivedvalue.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalicalderivedparameter.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalvalue.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalparameter.h   
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libicalicalderivedproperty.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalproperty.h    
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/pvl.h         
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalarray.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalcomponent.h   
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icaltimezone.h    
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalparser.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalmemory.h      
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalerror.h       
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.h 
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/sspm.h        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalmime.h        
   ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icallangbind.h
)




########### next target ###############

set(kcal_LIB_SRCS ${libversit_SRCS} ${libical_SRCS} ${libicalss_SRCS}
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

target_link_libraries(kcal  ${LIB_KDE3SUPPORT}  ktnef emailfunctions kmime kresources kabc  )

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
