project(ktnef)

include(TestBigEndian)
test_big_endian(CMAKE_WORDS_BIGENDIAN)
configure_file(config-ktnef.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef.h)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(pics)

set(ktnef_SRCS
  attachpropertydialog.cpp
  ktnefmain.cpp
  ktnefview.cpp
  main.cpp
  messagepropertydialog.cpp
  qwmf.cpp
)

kde4_add_app_icon(ktnef_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-ktnef.png")

ki18n_wrap_ui(ktnef_SRCS ui/attachpropertywidgetbase.ui)

add_executable(ktnef ${ktnef_SRCS})
target_link_libraries(ktnef KF5::Tnef KF5::KIOCore ) 


install(TARGETS ktnef ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS ktnef.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES ktnefui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktnef)

