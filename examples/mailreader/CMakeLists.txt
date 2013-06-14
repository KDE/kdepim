project(mailreader)

add_subdirectory(kmail-plasma)

set(mailreader_SRCS
  mailreader.cpp
  main.cpp
  mailreaderview.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${Boost_INCLUDE_DIRS}
)

kde4_add_ui_files(mailreader_SRCS mailreaderview.ui prefs_base.ui prefs_messagelist.ui)

kde4_add_kcfg_files(mailreader_SRCS settings.kcfgc)

kde4_add_executable(akonadimailreader ${mailreader_SRCS})

target_link_libraries(akonadimailreader
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_QGPGME_LIBS}
  akonadi_next
  kdepim
  kleo
  kpgp
  messagelist
  messageviewer
)

install(TARGETS akonadimailreader ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(PROGRAMS mailreader.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES mailreader.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES mailreaderui.rc DESTINATION ${DATA_INSTALL_DIR}/mailreader)
