kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkpgp ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kleopatra_LIB_SRCS
        directoryserviceswidgetbase.cpp
        adddirectoryservicedialog.cpp
        kdhorizontalline.cpp
	progressbar.cpp
	progressdialog.cpp
	keylistview.cpp
	keyselectiondialog.cpp
	keyrequester.cpp
	passphrasedialog.cpp
	keyapprovaldialog.cpp
	backendconfigwidget.cpp
	dnattributeorderconfigwidget.cpp
	cryptoconfigmodule.cpp
	cryptoconfigdialog.cpp
	directoryserviceswidget.cpp
	adddirectoryservicedialogimpl.cpp )

kde4_automoc(${kleopatra_LIB_SRCS})

kde4_add_ui_files(kleopatra_LIB_SRCS directoryserviceswidgetbase.ui adddirectoryservicedialog.ui)


########### install files ###############

install_files( /include/kleo FILES  )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I$(top_srcdir) \
#        -I$(top_srcdir)/libkpgp \
#	-I$(top_srcdir)/libkleo \
#	$(GPGME_CFLAGS) $(all_includes)
#
#noinst_LTLIBRARIES = libkleopatra_ui.la
#
#libkleopatra_ui_la_SOURCES = \
#        kdhorizontalline.cpp \
#	progressbar.cpp \
#	progressdialog.cpp \
#	keylistview.cpp \
#	keyselectiondialog.cpp \
#	keyrequester.cpp \
#	passphrasedialog.cpp \
#	keyapprovaldialog.cpp \
#	backendconfigwidget.cpp \
#	dnattributeorderconfigwidget.cpp \
#	cryptoconfigmodule.cpp \
#	cryptoconfigdialog.cpp \
#	directoryserviceswidgetbase.ui \
#	directoryserviceswidget.cpp \
#	adddirectoryservicedialog.ui \
#	adddirectoryservicedialogimpl.cpp
#
#kleodir = $(includedir)/kleo
#kleo_HEADERS = \
#        kdhorizontalline.h \
#	progressbar.h \
#	progressdialog.h \
#	keylistview.h \
#	keyselectiondialog.h \
#	keyrequester.h \
#	passphrasedialog.h \
#	keyapprovaldialog.h \
#	backendconfigwidget.h \
#	dnattributeorderconfigwidget.h
#
#METASOURCES = AUTO
