project(grantleethemeeditor)

add_definitions(-DTRANSLATION_DOMAIN=\"libgrantleethemeeditor\")


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

ecm_qt_declare_logging_category(libgrantleethemeeditor_SRCS HEADER grantleethemeeditor_debug.h IDENTIFIER GRANTLEETHEMEEDITOR_LOG CATEGORY_NAME log_grantleethemeeditor)

add_library(grantleethemeeditor ${libgrantleethemeeditor_SRCS})
generate_export_header(grantleethemeeditor BASE_NAME grantleethemeeditor)

target_include_directories(grantleethemeeditor PUBLIC "$<BUILD_INTERFACE:${grantleeditor_SOURCE_DIR};${grantleethemeeditor_SOURCE_DIR};${grantleethemeeditor_BINAY_DIR}>")

target_link_libraries(grantleethemeeditor
PUBLIC
  KF5::Archive 
PRIVATE
  KF5::KIOWidgets 
  KF5::PimTextEdit 
  KF5::PimCommon 
  KF5::ConfigWidgets
  KF5::I18n
)

install(FILES settings/grantleethemeeditor.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

set_target_properties(grantleethemeeditor PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS grantleethemeeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
