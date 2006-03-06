kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabinterfaces_LIB_SRCS 
   configurewidget.cpp 
   contacteditorwidget.cpp 
   core.cpp 
   extensionwidget.cpp 
   xxport.cpp )

kde4_automoc(${kabinterfaces_LIB_SRCS})

kde4_add_library(kabinterfaces SHARED ${kabinterfaces_LIB_SRCS})

target_link_libraries(kabinterfaces  ${LIB_KIO} kabc)

set_target_properties(kabinterfaces PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kabinterfaces )


########### install files ###############

install_files( /include/kaddressbook FILES configurewidget.h contacteditorwidget.h core.h                       extensionwidget.h xxport.h )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES kaddressbook_contacteditorwidget.desktop                     kaddressbook_extension.desktop kaddressbook_xxport.desktop )

kde4_footer()



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
