project(mailreader)

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

ki18n_wrap_ui(mailreader_SRCS mailreaderview.ui prefs_base.ui prefs_messagelist.ui)

kconfig_add_kcfg_files(mailreader_SRCS settings.kcfgc)

add_executable(akonadimailreader ${mailreader_SRCS})

target_link_libraries(akonadimailreader
  KF5::AkonadiMime
  KF5::AkonadiCore
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
install(FILES mailreaderui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/mailreader)
