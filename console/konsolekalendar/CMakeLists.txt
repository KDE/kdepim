kde4_header()

add_subdirectory( pixmaps ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkcal ${CMAKE_SOURCE_DIR}/libemailfunctions ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(konsolekalendar_SRCS 
   konsolekalendarepoch.cpp 
   konsolekalendardelete.cpp 
   konsolekalendarchange.cpp 
   konsolekalendarvariables.cpp 
   konsolekalendaradd.cpp 
   konsolekalendarexports.cpp 
   konsolekalendar.cpp 
   stdcalendar.cpp 
   main.cpp )

kde4_automoc(${konsolekalendar_SRCS})

kde4_add_executable(konsolekalendar ${konsolekalendar_SRCS})

target_link_libraries(konsolekalendar  ${QT_AND_KDECORE_LIBS} kcal kdepim )

install_targets(/bin konsolekalendar )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES konsolekalendar.desktop )
install_files( ${APPLNK_INSTALL_DIR}/Utilities FILES  )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = pixmaps
#
#INCLUDES= -I$(top_srcdir)/libkcal \
#          -I$(top_srcdir)/libemailfunctions \
#          -I$(top_srcdir) $(all_includes)
#
#METASOURCES = AUTO
#
#bin_PROGRAMS = konsolekalendar
#
#konsolekalendar_SOURCES = konsolekalendarepoch.cpp konsolekalendardelete.cpp \
#  konsolekalendarchange.cpp konsolekalendarvariables.cpp \
#  konsolekalendaradd.cpp konsolekalendarexports.cpp konsolekalendar.cpp \
#  stdcalendar.cpp main.cpp 
#konsolekalendar_LDADD = $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la
#konsolekalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#appsdir = $(kde_appsdir)/Utilities
#xdg_apps_DATA = konsolekalendar.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/konsolekalendar.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
#
#konsolekalendar.o: ../libkcal/htmlexportsettings.h
