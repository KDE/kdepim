kde4_header()

add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmailcvt_SRCS 
   kimportpage.cpp 
   kselfilterpage.cpp 
   filters.cxx 
   filter_oe.cxx 
   kmailcvt.cpp 
   main.cpp 
   filter_pmail.cxx 
   filter_plain.cxx 
   filter_outlook.cxx 
   filter_mbox.cxx 
   filter_evolution.cxx 
   filter_mailapp.cxx 
   filter_evolution_v2.cxx 
   filter_opera.cxx 
   filter_thunderbird.cxx 
   filter_kmail_maildir.cxx 
   filter_sylpheed.cxx 
   filter_thebat.cxx 
   filter_lnotes.cxx )

kde4_automoc(${kmailcvt_SRCS})

kde4_add_ui3_files(kmailcvt_SRCS kimportpagedlg.ui kselfilterpagedlg.ui )

kde4_add_executable(kmailcvt ${kmailcvt_SRCS})

target_link_libraries(kmailcvt  ${QT_AND_KDECORE_LIBS} kde3support )

install_targets(/bin kmailcvt )


########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#
#bin_PROGRAMS = kmailcvt
#kmailcvt_SOURCES = kimportpagedlg.ui kimportpage.cpp kselfilterpagedlg.ui \
#                   kselfilterpage.cpp filters.cxx filter_oe.cxx kmailcvt.cpp \
#                   main.cpp filter_pmail.cxx filter_plain.cxx filter_outlook.cxx \
#                   filter_mbox.cxx filter_evolution.cxx filter_mailapp.cxx \
#                   filter_evolution_v2.cxx filter_opera.cxx filter_thunderbird.cxx \
#                   filter_kmail_maildir.cxx filter_sylpheed.cxx filter_thebat.cxx \
#                   filter_lnotes.cxx
#                
#kmailcvt_LDADD   = $(LIB_KFILE) $(LIB_KDE3SUPPORT)
#
#EXTRA_DIST = main.cpp kmailcvt.cpp kimportpage.cpp kimportpage.h \
#             kselfilterpage.cpp kselfilterpage.h kmailcvt.h \
#             kmailcvt.desktop filter_oe.cxx filter_oe.hxx \
#             filters.cxx filters.hxx \
#             PAB_format.html sv9429424.gif \
#             filter_pmail.cxx filter_pmail.hxx \
#             filter_plain.cxx filter_plain.hxx \
#             filter_mbox.cxx filter_mbox.hxx \
#             filter_evolution.cxx filter_evolution.hxx \
#             filter_mailapp.hxx filter_mailapp.cxx \
#             filter_opera.hxx filter_opera.cxx \
#             filter_evolution_v2.hxx filter_evolution_v2.cxx \
#             filter_thunderbird.hxx filter_thunderbird.cxx \
#             filter_kmail_maildir.hxx filter_kmail_maildir.cxx \
#             filter_sylpheed.hxx filter_sylpheed.cxx \
#             filter_thebat.hxx filter_thebat.cxx filter_lnotes.hxx filter_lnotes.cxx
#
#install-data-local: uninstall.desktop
#	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Utilities
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Utilities/kmailcvt.desktop
#
#SUBDIRS = pics
#
## set the include path for X, qt and KDE
#INCLUDES= $(all_includes)
#
#kmailcvt_METASOURCES = AUTO
#
## the library search path. 
#kmailcvt_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#KDE_ICON=AUTO
#
#messages: rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/kmailcvt.pot; \
#	fi
#
