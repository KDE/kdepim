project(kmailcvt)
add_subdirectory( pics ) 




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

qt4_add_dbus_interfaces(kmailcvt_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.kmail.xml )

kde4_automoc(${kmailcvt_SRCS})


kde4_add_ui_files(kmailcvt_SRCS kselfilterpagedlg.ui kimportpagedlg.ui)

kde4_add_executable(kmailcvt ${kmailcvt_SRCS})

target_link_libraries(kmailcvt  ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kmailcvt  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############


kde4_install_icons( ${ICON_INSTALL_DIR}   )


