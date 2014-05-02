project(kpapplet)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
)

kde_enable_exceptions()

set(kpapplet_SRCS
    kpapplet.cpp
    kpdialog.cpp
)

add_library(plasma_applet_kpapplet MODULE ${kpapplet_SRCS})

target_link_libraries(
  plasma_applet_kpapplet
  KF5::KIOCore
  KF5::AkonadiCore
  akonadi_next
  kdepim
  kleo
  kpgp
  messagelist
)

install(TARGETS plasma_applet_kpapplet DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kpapplet.desktop DESTINATION ${SERVICES_INSTALL_DIR})
