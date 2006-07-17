project(knode)
add_subdirectory( pics ) 
add_subdirectory( filters ) 

include_directories( ${KDE4_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkmime ${CMAKE_SOURCE_DIR}/libkdepim ${CMAKE_SOURCE_DIR}/libkpgp )


########### next target ###############

set(knodecommon_LIB_SRCS 
   knconfigmanager.cpp 
   knconfig.cpp 
   knconfigwidgets.cpp 
   settings.cpp 
   scheduler.cpp 
   knjobdata.cpp 
   mailsendjob.cpp 
   nntpjobs.cpp 
   knaccountmanager.cpp 
   kncollection.cpp 
   kncollectionviewitem.cpp 
   knserverinfo.cpp 
   knnntpaccount.cpp 
   kngroupmanager.cpp 
   knarticlecollection.cpp 
   kngroup.cpp 
   kngroupbrowser.cpp 
   kngroupselectdialog.cpp 
   kngroupdialog.cpp 
   kngrouppropdlg.cpp 
   knfoldermanager.cpp 
   knfolder.cpp 
   knmemorymanager.cpp 
   kncleanup.cpp 
   knconvert.cpp 
   knarticlemanager.cpp 
   knarticle.cpp 
   kndisplayedheader.cpp 
   knsourceviewwindow.cpp 
   knarticlewindow.cpp 
   knhdrviewitem.cpp 
   kncomposer.cpp 
   knarticlefactory.cpp 
   knfiltermanager.cpp 
   knstatusfilter.cpp 
   knstringfilter.cpp 
   knrangefilter.cpp 
   knarticlefilter.cpp 
   knfilterconfigwidget.cpp 
   knfilterdialog.cpp 
   knsearchdialog.cpp 
   utilities.cpp 
   knscoring.cpp 
   headerview.cpp 
   knmainwidget.cpp 
   aboutdata.cpp 
   knglobals.cpp 
   kncollectionview.cpp 
   articlewidget.cpp 
   csshelper.cpp
   keditcl1.cpp keditcl2.cpp )

kde4_automoc(${knodecommon_LIB_SRCS})

kde4_add_ui_files(knodecommon_LIB_SRCS 
   smtpaccountwidget_base.ui 
   nntpaccountdialog_base.ui 
   nntpaccountlistwidget_base.ui 
   readnewsgeneralwidget_base.ui 
   readnewsnavigationwidget_base.ui 
   readnewsviewerwidget_base.ui 
   postnewstechnicalwidget_base.ui 
   postnewscomposerwidget_base.ui )

kde4_add_kcfg_files(knodecommon_LIB_SRCS settings_base.kcfgc )

kde4_add_library(knodecommon SHARED ${knodecommon_LIB_SRCS})

target_link_libraries(knodecommon  ${KDE4_KDECORE_LIBS} kdeprint kwalletclient kmime kpgp kdepim kutils khtml )

set_target_properties(knodecommon PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS knodecommon  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(kcm_knode_PART_SRCS knconfigpages.cpp )

kde4_automoc(${kcm_knode_PART_SRCS})

kde4_add_plugin(kcm_knode ${kcm_knode_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_knode )

target_link_libraries(kcm_knode  ${KDE4_KDECORE_LIBS} knodecommon )

install(TARGETS kcm_knode  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(knodepart_PART_SRCS knode_part.cpp )

kde4_automoc(${knodepart_PART_SRCS})

kde4_add_plugin(knodepart WITH_PREFIX ${knodepart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} knodepart )

target_link_libraries(knodepart  ${KDE4_KDECORE_LIBS} knodecommon )

install(TARGETS knodepart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(knode_SRCS knode.cpp knapplication.cpp main.cpp )

kde4_automoc(${knode_SRCS})

kde4_add_executable(knode ${knode_SRCS})

target_link_libraries(knode ${KDE4_EMAILFUNCTIONS_LIBS} ${KDE4_KDECORE_LIBS} knodecommon )

install(TARGETS knode  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES knewsservice.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES KNode.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES knode_config_identity.desktop knode_config_accounts.desktop knode_config_appearance.desktop knode_config_read_news.desktop knode_config_post_news.desktop knode_config_privacy.desktop knode_config_cleanup.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES headers.rc knodeui.rc knreaderui.rc kncomposerui.rc  DESTINATION ${DATA_INSTALL_DIR}/knode)

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#KDE_CXXFLAGS = $(USE_THREADS)
#
#INCLUDES=	-I$(top_srcdir)/libkmime \
#		-I$(top_srcdir)/libkdepim \
#		-I$(top_srcdir)/libkpgp \
#		-I$(top_srcdir)/libemailfunctions \
#		-I$(top_srcdir) \
#		 $(all_includes)
#
#lib_LTLIBRARIES = libknodecommon.la
#
#kde_module_LTLIBRARIES = kcm_knode.la libknodepart.la
#libknodepart_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries) $(KDE_RPATH) $(KDE_PLUGIN)
#libknodepart_la_LIBADD	= libknodecommon.la
#
#SUBDIRS = pics filters
#
#bin_PROGRAMS = knode
#
#knode_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#knode_LDADD   = libknodecommon.la
#
#kcm_knode_la_SOURCES = knconfigpages.cpp
#kcm_knode_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_knode_la_LIBADD = libknodecommon.la $(LIB_KDECORE)
#knconfigpages.lo: nntpaccountdialog_base.h smtpaccountwidget_base.h
#
#libknodecommon_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
#libknodecommon_la_LIBADD = -lkdeprint -lkwalletclient $(top_builddir)/libkmime/libkmime.la $(top_builddir)/libkpgp/libkpgp.la $(top_builddir)/libkdepim/libkdepim.la  $(LIB_KABC) $(LIB_KFILE) $(LIB_KUTILS) $(LIBRESOLV) $(LIB_KHTML)
#libknodecommon_la_SOURCES =   knconfigmanager.cpp \
#                  knconfig.cpp \
#                  knconfigwidgets.cpp \
#                  settings_base.kcfgc \
#                  settings.cpp \
#                  \
#                  scheduler.cpp \
#                  knjobdata.cpp \
#                  mailsendjob.cpp \
#                  nntpjobs.cpp \
#                  \
#                  knaccountmanager.cpp \
#                  kncollection.cpp \
#                  kncollectionviewitem.cpp \
#                  knserverinfo.cpp \
#                  knnntpaccount.cpp \
#                  \
#                  kngroupmanager.cpp \
#                  knarticlecollection.cpp \
#                  kngroup.cpp \
#                  kngroupbrowser.cpp \
#                  kngroupselectdialog.cpp \
#                  kngroupdialog.cpp \
#                  kngrouppropdlg.cpp \
#                  \
#                  knfoldermanager.cpp \
#                  knfolder.cpp \
#                  \
#                  knmemorymanager.cpp \
#                  kncleanup.cpp \
#                  knconvert.cpp \
#                  \
#                  knarticlemanager.cpp \
#                  knarticle.cpp \
#                  kndisplayedheader.cpp \
#                  knsourceviewwindow.cpp \
#                  knarticlewindow.cpp \
#                  knhdrviewitem.cpp \
#                  \
#                  kncomposer.cpp \
#                  knarticlefactory.cpp \
#                  \
#                  knfiltermanager.cpp \
#                  knstatusfilter.cpp \
#                  knstringfilter.cpp \
#                  knrangefilter.cpp \
#                  knarticlefilter.cpp \
#                  knfilterconfigwidget.cpp \
#                  knfilterdialog.cpp \
#                  knsearchdialog.cpp \
#                  \
#                  utilities.cpp \
#                  \
#                  knscoring.cpp \
#                  \
#                  headerview.cpp \
#		  knmainwidget.cpp    \
#		  aboutdata.cpp \
#		  knglobals.cpp \
#		  knodecomposeriface.skel \
#		  knodeiface.skel \
#		  kncollectionview.cpp \
#                  articlewidget.cpp \
#                  csshelper.cpp \
#                  smtpaccountwidget_base.ui \
#                  nntpaccountdialog_base.ui \
#                  nntpaccountlistwidget_base.ui \
#                  readnewsgeneralwidget_base.ui \
#                  readnewsnavigationwidget_base.ui \
#                  readnewsviewerwidget_base.ui \
#                  postnewstechnicalwidget_base.ui \
#                  postnewscomposerwidget_base.ui
#libknodecommon_la_COMPILE_FIRST = nntpaccountdialog_base.ui smtpaccountwidget_base.h
#
#knode_SOURCES = knode.cpp knapplication.cpp main.cpp
#
#libknodepart_la_SOURCES = knode_part.cpp
#
#
#noinst_HEADERS =  knconfigmanager.h \
#                  knconfig.h \
#                  knconfigwidgets.h \
#                  settings.h \
#                  \
#                  scheduler.h \
#                  knjobdata.h \
#                  mailsendjob.h \
#                  nntpjobs.h \
#                  \
#                  knaccountmanager.h \
#                  kncollection.h \
#                  kncollectionviewitem.h \
#                  knserverinfo.h \
#                  knnntpaccount.h \
#                  \
#                  kngroupmanager.h \
#                  knarticlecollection.h \
#                  kngroup.h \
#                  kngroupbrowser.h \
#                  kngroupselectdialog.h \
#                  kngroupdialog.h \
#                  kngrouppropdlg.h \
#                  \
#                  knfoldermanager.h \
#                  knfolder.h \
#                  \
#                  knmemorymanager.h \
#                  kncleanup.h \
#                  knconvert.h \
#                  \
#                  knarticlemanager.h \
#                  knarticle.h \
#                  kndisplayedheader.h \
#                  knsourceviewwindow.h \
#                  knarticlewindow.h \
#                  knhdrviewitem.h \
#                  \
#                  kncomposer.h \
#                  knarticlefactory.h \
#                  \
#                  knfiltermanager.h \
#                  knstatusfilter.h \
#                  knstringfilter.h \
#                  knrangefilter.h \
#                  knarticlefilter.h \
#                  knfilterconfigwidget.h \
#                  knfilterdialog.h \
#                  knsearchdialog.h \
#                  \
#                  utilities.h \
#                  \
#                  knscoring.h \
#                  \
#                  headerview.h \
#                  knode.h \
#                  knapplication.h \
#                  resource.h \
#                  knglobals.h \
#		  aboutdata.h \
#		  knmainwidget.h \
#		  kncollectionview.h \
#                  articlewidget.h \
#                  csshelper.h
#
#
#knode_METASOURCES = AUTO
#
#xdg_apps_DATA = KNode.desktop
#
#rcdir = $(kde_datadir)/knode
#rc_DATA = headers.rc knodeui.rc knreaderui.rc kncomposerui.rc
#
#knewsprotocol_DATA = knewsservice.protocol
#knewsprotocoldir = $(kde_servicesdir)
#
#kde_services_DATA = knode_config_identity.desktop knode_config_accounts.desktop knode_config_appearance.desktop knode_config_read_news.desktop knode_config_post_news.desktop knode_config_privacy.desktop knode_config_cleanup.desktop
#
#KDE_ICON = AUTO
#
#EXTRA_DIST = KNode.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/knode.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
