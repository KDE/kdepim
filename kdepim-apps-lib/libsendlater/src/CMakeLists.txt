
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

add_library( KF5SendLater ${sendlaterlib_SRCS} )
generate_export_header(KF5SendLater BASE_NAME sendlater)
add_library(KF5::SendLater ALIAS KF5SendLater)
target_link_libraries(KF5SendLater
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
target_include_directories(KF5SendLater INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/SendLater/;${KDE_INSTALL_INCLUDEDIR_KF5}/sendlater>")

target_include_directories(KF5SendLater PUBLIC "$<BUILD_INTERFACE:${sendlater_SOURCE_DIR}/src;${sendlater_BINARY_DIR}/src>")


set_target_properties(KF5SendLater PROPERTIES
    VERSION ${SENDLATER_VERSION_STRING}
    SOVERSION ${SENDLATER_SOVERSION}
    EXPORT_NAME SendLater
)

install(TARGETS
    KF5SendLater
    EXPORT KF5SendLaterTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(SendLater_Camelcase_HEADERS
  HEADER_NAMES
  SendLaterInfo
  SendLaterDialog
  SendLaterUtil
  REQUIRED_HEADERS SendLater_HEADERS
  PREFIX SendLater
)

ecm_generate_pri_file(BASE_NAME SendLater
    LIB_NAME KF5SendLater
    DEPS "AkonadiCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/SendLater
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kdepim-apps-lib_SOURCE_DIR})
install(FILES
    ${SendLater_Camelcase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/SendLater
    COMPONENT Devel
)

install(FILES
    ${SendLater_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/sendlater_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/sendlateragentsettings.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/sendlater
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()
