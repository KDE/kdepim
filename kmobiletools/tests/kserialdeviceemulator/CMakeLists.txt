
########### next target ###############

set(kserialdeviceemulator_SRCS 
   main.cpp 
   kserialdeviceemulator.cpp 
   kserialdeviceemulatorwidget.cpp 
   commandslist.cpp )


kde4_add_ui_files(kserialdeviceemulator_SRCS kserialdeviceemulatorwidgetbase.ui )

kde4_add_executable(kserialdeviceemulator ${kserialdeviceemulator_SRCS})

target_link_libraries(kserialdeviceemulator  ${KDE4_KDEUI_LIBS} kmobiletools )

install(TARGETS kserialdeviceemulator DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  kserialdeviceemulatorui.rc DESTINATION ${DATA_INSTALL_DIR}/kserialdeviceemulator )
install( FILES  kserialdeviceemulator.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR} )



#original Makefile.am contents follow:

## set the include path for X, qt and KDE
#INCLUDES = $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = kserialdeviceemulator.h commandslist.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kserialdeviceemulator.pot
#
#KDE_ICON = AUTO
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = kserialdeviceemulator
#
## the application source, library search path, and link libraries
#kserialdeviceemulator_SOURCES = main.cpp kserialdeviceemulator.cpp \
#	kserialdeviceemulatorwidget.cpp kserialdeviceemulatorwidgetbase.ui commandslist.cpp
#kserialdeviceemulator_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kserialdeviceemulator_LDADD = -lkmobiletools $(LIB_KDEUI)
#
## this is where the desktop file will go 
#shelldesktopdir   = $(kde_appsdir)/Utilities
#shelldesktop_DATA = kserialdeviceemulator.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir   = $(kde_datadir)/kserialdeviceemulator
#shellrc_DATA = kserialdeviceemulatorui.rc
#
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
