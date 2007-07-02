
########### next target ###############

set(kmtsetup_SRCS main.cpp kmtsetupwidget.cpp )

kde4_automoc(${kmtsetup_SRCS})

kde4_add_ui_files(kmtsetup_SRCS kmtsetupwidgetbase.ui )

kde4_add_executable(kmtsetup ${kmtsetup_SRCS})

target_link_libraries(kmtsetup  ${KDE4_KIO_LIBS} )

install(TARGETS kmtsetup DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  kmtsetup.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES  kmtsetupui.rc kmobilewizard.png DESTINATION ${DATA_INSTALL_DIR}/kmtsetup )

kde4_install_icons( ${ICON_INSTALL_DIR} )



#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
#
## set the include path for X, qt and KDE
#INCLUDES = $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = kmtsetupwidget.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kmtsetup.pot
#
#KDE_ICON = AUTO
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = kmtsetup
#
## the application source, library search path, and link libraries
#kmtsetup_SOURCES = main.cpp kmtsetupwidget.cpp kmtsetupwidgetbase.ui
#
#kmtsetup_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kmtsetup_LDADD = -lkio $(LIB_KDEUI)
#
## this is where the desktop file will go 
#xdg_apps_DATA = kmtsetup.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir   = $(kde_datadir)/kmtsetup
#shellrc_DATA = kmtsetupui.rc kmobilewizard.png
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
