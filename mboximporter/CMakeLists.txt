project(mboximporter)


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
set(mboximporter_SRCS
  main.cpp
  mboxmainwindow.cpp
  mboximporterinfogui.cpp
  mboximportwidget.cpp
  mboximportkernel.cpp
)

ki18n_wrap_ui(mboximporter_SRCS 
                  ui/mboximportwidget.ui
)

include_directories(${CMAKE_SOURCE_DIR}/mailimporter ${CMAKE_SOURCE_DIR}/mailcommon ${CMAKE_SOURCE_DIR}/messagecomposer)

add_executable(mboximporter ${mboximporter_SRCS})

target_link_libraries(mboximporter
  KF5::AkonadiCore
  KF5::IdentityManagement
  mailimporter
  mailcommon
)

install(TARGETS mboximporter  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(PROGRAMS org.kde.mboximporter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
