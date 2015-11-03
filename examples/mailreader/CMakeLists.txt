project(mailreader)

set(mailreader_SRCS
  mailreader.cpp
  main.cpp
  mailreaderview.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
)

ki18n_wrap_ui(mailreader_SRCS mailreaderview.ui prefs_base.ui prefs_messagelist.ui)

kconfig_add_kcfg_files(mailreader_SRCS settings.kcfgc)

add_executable(akonadimailreader ${mailreader_SRCS})

target_link_libraries(akonadimailreader
  KF5::AkonadiMime
  KF5::AkonadiCore
  KF5::XmlGui
  KF5::IconThemes  
  KF5::Libkdepim
  KF5::Libkleo
  KF5::MessageList
  KF5::MessageViewer
)

install(TARGETS akonadimailreader ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(PROGRAMS org.kde.mailreader.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES mailreader.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES mailreaderui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/mailreader)
