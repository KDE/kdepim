kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkdepim ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kgantt_LIB_SRCS 
   KGantt.cpp 
   xQGanttBarViewPort.cpp 
   xQGanttListViewPort.cpp 
   KGanttItem.cpp 
   KGanttRelation.cpp 
   xQGanttBarViewPort_Events.cpp 
   xQGanttBarView.cpp 
   xQGanttListView.cpp 
   KGanttBarConfig.cpp )

kde4_automoc(${kgantt_LIB_SRCS})

kde4_add_library(kgantt SHARED ${kgantt_LIB_SRCS})

target_link_libraries(kgantt  ${LIB_KDE3SUPPORT} )

set_target_properties(kgantt PROPERTIES VERSION 0.0.2 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} kgantt )


########### install files ###############

install_files( /include/kgantt FILES KGantt.h  KGanttItem.h  KGanttRelation.h  xQGanttBarView.h   xQGanttBarViewPort.h  xQGanttListView.h  xQGanttListViewPort.h KGanttBarConfig.h )

kde4_footer()



#original Makefile.am contents follow:

#
#lib_LTLIBRARIES = libkgantt.la
#libkgantt_la_SOURCES = KGantt.cpp xQGanttBarViewPort.cpp  \
#xQGanttListViewPort.cpp \
#KGanttItem.cpp KGanttRelation.cpp \
#xQGanttBarViewPort_Events.cpp \
#xQGanttBarView.cpp  xQGanttListView.cpp KGanttBarConfig.cpp
#
#libkgantt_la_LDFLAGS = -version-info 0:2:0 $(all_libraries) -no-undefined
#libkgantt_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) 
#
## set the include path for X, qt and KDE
#INCLUDES= $(all_includes)
#
#METASOURCES = AUTO
#
#kganttincludedir = $(includedir)/kgantt
#kganttinclude_HEADERS = KGantt.h \
#KGanttItem.h  KGanttRelation.h \
#xQGanttBarView.h  \
#xQGanttBarViewPort.h  xQGanttListView.h  xQGanttListViewPort.h KGanttBarConfig.h
#
#messages:  rc.cpp
#	 $(XGETTEXT) *.cpp -o $(podir)/kgantt.pot
