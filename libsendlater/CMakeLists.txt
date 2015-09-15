project(libsendlater)

add_definitions(-DTRANSLATION_DOMAIN=\"libsendlater\")

set(sendlater_common_SRCS)
ecm_qt_declare_logging_category(sendlater_common_SRCS HEADER libsendlater_debug.h IDENTIFIER LIBSENDLATER_LOG CATEGORY_NAME log_libsendlater)

set(sendlaterlib_SRCS 
    ${sendlater_common_SRCS}
    sendlaterinfo.cpp
    sendlaterdialog.cpp
    sendlaterutil.cpp
    sendlatertimedatewidget.cpp
)

kconfig_add_kcfg_files(sendlaterlib_SRCS
    settings/sendlateragentsettings.kcfgc 
  )

ki18n_wrap_ui(sendlaterlib_SRCS ui/sendlaterwidget.ui)

add_library( sendlater ${sendlaterlib_SRCS} )
generate_export_header(sendlater BASE_NAME sendlater)
target_link_libraries(sendlater
  PUBLIC
    KF5::AkonadiCore
  PRIVATE
    KF5::ConfigCore
    KF5::I18n
    KF5::WidgetsAddons
    KF5::ConfigGui
    Qt5::DBus
    Qt5::Widgets
)
target_include_directories(sendlater PUBLIC "$<BUILD_INTERFACE:${sendlateragent_SOURCE_DIR};${sendlateragent_BINARY_DIR}>")
set_target_properties( sendlater PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )

install( TARGETS sendlater ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

