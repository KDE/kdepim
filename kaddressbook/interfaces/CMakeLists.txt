



########### next target ###############

set(kabinterfaces_LIB_SRCS 
   configurewidget.cpp 
   contacteditorwidget.cpp 
   core.cpp 
   extensionwidget.cpp 
   xxport.cpp )

kde4_automoc(${kabinterfaces_LIB_SRCS})

kde4_add_library(kabinterfaces SHARED ${kabinterfaces_LIB_SRCS})

target_link_libraries(kabinterfaces  ${KDE4_KIO_LIBS} kabc)

set_target_properties(kabinterfaces PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kabinterfaces  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES configurewidget.h contacteditorwidget.h core.h extensionwidget.h xxport.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kaddressbook)
install( FILES kaddressbook_contacteditorwidget.desktop kaddressbook_extension.desktop kaddressbook_xxport.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libkabinterfaces.la
#
#libkabinterfaces_la_SOURCES = configurewidget.cpp contacteditorwidget.cpp \
#                              core.cpp extensionwidget.cpp xxport.cpp
#libkabinterfaces_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libkabinterfaces_la_LIBADD = $(LIB_KABC)
#
#kabincludedir = $(includedir)/kaddressbook
#kabinclude_HEADERS = configurewidget.h contacteditorwidget.h core.h \
#                     extensionwidget.h xxport.h
#
#METASOURCES = AUTO
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kaddressbook_contacteditorwidget.desktop \
#                   kaddressbook_extension.desktop kaddressbook_xxport.desktop
