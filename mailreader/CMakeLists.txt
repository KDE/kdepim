
set(mailreader_SRCS
   mailreader.cpp
   main.cpp
   mailreaderview.cpp
 )

include_directories(
    ${CMAKE_SOURCE_DIR}/libmailreader
    ${Boost_INCLUDE_DIRS}
)

kde4_add_ui_files(mailreader_SRCS mailreaderview.ui prefs_base.ui)

kde4_add_kcfg_files(mailreader_SRCS settings.kcfgc )

kde4_add_executable(akonadimailreader ${mailreader_SRCS})

target_link_libraries(akonadimailreader mailreader kdepim kpgp kleo akonadi-kde ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS}    ${KDE4_KTNEF_LIBRARY}
 ${QGPGME_LIBRARIES}
)

install(TARGETS akonadimailreader ${INSTALL_TARGETS_DEFAULT_ARGS} )



########### install files ###############

install( FILES mailreader.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES mailreader.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES mailreaderui.rc  DESTINATION  ${DATA_INSTALL_DIR}/mailreader )
