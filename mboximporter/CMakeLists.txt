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

add_executable(mboximporter ${mboximporter_SRCS})

target_link_libraries(mboximporter
PRIVATE
  KF5::AkonadiCore
  KF5::IdentityManagement
  KF5::MailImporter
  mailcommon
  KF5::Completion
)

install(TARGETS mboximporter  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(PROGRAMS org.kde.mboximporter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
