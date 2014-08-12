project(grantleethemeeditor)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DTRANSLATION_DOMAIN=\"libgrantleethemeeditor\")

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
)


set(libgrantleethemeeditor_SRCS 
    newthemedialog.cpp 
    themeeditortabwidget.cpp 
    managethemes.cpp 
    themesession.cpp 
    editorwidget.cpp 
    desktopfilepage.cpp 
    editorpage.cpp
    previewwidget.cpp
    configurewidget.cpp
    grantleeplaintexteditor.cpp
)

kconfig_add_kcfg_files(libgrantleethemeeditor_SRCS
  settings/globalsettings_base.kcfgc
)

ki18n_wrap_ui(libgrantleethemeeditor_SRCS
    ui/configurewidget.ui
)


add_library(grantleethemeeditor ${LIBRARY_TYPE} ${libgrantleethemeeditor_SRCS})
generate_export_header(grantleethemeeditor BASE_NAME grantleethemeeditor)

target_link_libraries(grantleethemeeditor    KF5::KIOCore KF5::PimTextEdit pimcommon
)

set_target_properties(grantleethemeeditor PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS grantleethemeeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
