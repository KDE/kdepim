project(kpapplet)

find_package(KDE4 REQUIRED)
find_package(Akonadi REQUIRED)
find_package(KdepimLibs REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${AKONADI_INCLUDE_DIR}
  ${KDE4_INCLUDES}
  ${KDEPIMLIBS_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}

)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(kpapplet_SRCS
    kpapplet.cpp
    kpdialog.cpp
)

kde4_add_plugin(plasma_applet_kpapplet ${kpapplet_SRCS})

target_link_libraries(
  plasma_applet_kpapplet
  ${KDE4_PLASMA_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  akonadi_next
  kdepim
  kleo
  kpgp
  messagelist
)

install(TARGETS plasma_applet_kpapplet DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kpapplet.desktop DESTINATION ${SERVICES_INSTALL_DIR})
