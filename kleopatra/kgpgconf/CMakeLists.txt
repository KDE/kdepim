project(kgpgconf)

include_directories( ${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme )
add_definitions ( -D_ASSUAN_ONLY_GPG_ERRORS -DKDE_DEFAULT_DEBUG_AREA=5154 )
remove_definitions ( -DQT_NO_STL )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(_kgpgconf_SRCS
    configreader.cpp
    configwriter.cpp
    configuration.cpp
    exception.cpp
    main.cpp
    mainwindow.cpp
)

kde4_add_ui_files( _kgpgconf_SRCS mainwidget.ui )


# todo: more appropriate icon?
kde4_add_app_icon(_kgpgconf_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/preferences-desktop-cryptography.png")

kde4_add_executable(kgpgconf ${_kgpgconf_SRCS} )
target_link_libraries(kgpgconf ${QGPGME_LIBRARIES} ${KDE4_KDEUI_LIBS} )

install(TARGETS kgpgconf ${INSTALL_TARGETS_DEFAULT_ARGS})

