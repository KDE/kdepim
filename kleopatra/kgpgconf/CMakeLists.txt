project(kgpgconf)

include_directories(${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme)
add_definitions(-D_ASSUAN_ONLY_GPG_ERRORS)

kde_enable_exceptions()

set(_kgpgconf_SRCS
  configreader.cpp
  configwriter.cpp
  configuration.cpp
  exception.cpp
  main.cpp
  mainwindow.cpp
)

ki18n_wrap_ui(_kgpgconf_SRCS mainwidget.ui)

# todo: more appropriate icon?
kde4_add_app_icon(_kgpgconf_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/preferences-desktop-cryptography.png")

add_executable(kgpgconf ${_kgpgconf_SRCS})

target_link_libraries(kgpgconf ${QGPGME_LIBRARIES} Qt5::Core KF5::Gpgmepp KF5::I18n Qt5::Widgets)

install(TARGETS kgpgconf ${INSTALL_TARGETS_DEFAULT_ARGS})
