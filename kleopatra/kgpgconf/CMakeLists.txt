project(kgpgconf)

include_directories( ${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme )
add_definitions ( ${KDE4_ENABLE_EXCEPTIONS} -D_ASSUAN_ONLY_GPG_ERRORS -UQT_NO_STL )

set(_kgpgconf_SRCS
    configreader.cpp
    configwriter.cpp
    configuration.cpp
    main.cpp
    mainwindow.cpp
)

kde4_add_ui_files( _kgpgconf_SRCS mainwidget.ui )

kde4_add_executable(kgpgconf ${_kgpgconf_SRCS} )
target_link_libraries(kgpgconf ${KDE4_KDEUI_LIBS} )

install(TARGETS kgpgconf DESTINATION ${BIN_INSTALL_DIR} )

