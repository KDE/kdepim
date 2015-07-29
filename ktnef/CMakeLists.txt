project(ktnef)

include(TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)
configure_file(config-ktnef.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef.h)
add_definitions(-DTRANSLATION_DOMAIN=\"ktnef\")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(pics)

set(ktnef_SRCS
  ktnef_debug.cpp
  attachpropertydialog.cpp
  ktnefmain.cpp
  ktnefview.cpp
  main.cpp
  messagepropertydialog.cpp
  qwmf.cpp
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hicolor/*-apps-ktnef.png")
ecm_add_app_icon(ktnef_SRCS ICONS ${ICONS_SRCS})

ki18n_wrap_ui(ktnef_SRCS ui/attachpropertywidgetbase.ui)

add_executable(ktnef ${ktnef_SRCS})
target_link_libraries(ktnef KF5::Tnef KF5::DBusAddons) 


install(TARGETS ktnef ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.ktnef.desktop DESTINATION ${KDE_INSTALL_APPDIR})

install(FILES ktnefui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/ktnef)

