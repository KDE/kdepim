
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


########### next target ###############

set(kdepimdbusinterfaces_LIB_SRCS
    urihandler.cpp
    reminderclient.cpp
    kdepimdbusinterface_debug.cpp
)

ecm_qt_declare_logging_category(kdepimdbusinterfaces_LIB_SRCS HEADER kdepimdbusinterface_debug.h IDENTIFIER KDEPIMDBUSINTERFACE_LOG CATEGORY_NAME log_kdepimdbusinterface)


qt5_generate_dbus_interface(dbusinterfaces/kmkernel.h org.kde.kmail.kmail.xml OPTIONS -a)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS dbusinterfaces/org.kde.korganizer.KOrgac.xml)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS dbusinterfaces/org.kde.korganizer.Korganizer.xml)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.kmail.xml)

add_library(KF5KdepimDBusInterfaces ${kdepimdbusinterfaces_LIB_SRCS})
generate_export_header(KF5KdepimDBusInterfaces BASE_NAME kdepimdbusinterfaces)
add_library(KF5::KdepimDBusInterfaces ALIAS KF5KdepimDBusInterfaces)

add_dependencies(KF5KdepimDBusInterfaces kmail_xml)

target_link_libraries(KF5KdepimDBusInterfaces
PUBLIC
    KF5::AkonadiCore
PRIVATE
    KF5::AkonadiContact
    KF5::KIOWidgets
    Qt5::DBus
)

target_include_directories(KF5KdepimDBusInterfaces INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KdepimDBusInterfaces/;${KDE_INSTALL_INCLUDEDIR_KF5}/kdepimdbusinterfaces>")

target_include_directories(KF5KdepimDBusInterfaces PUBLIC "$<BUILD_INTERFACE:${kdepimdbusinterfaces_SOURCE_DIR}/src;${kdepimdbusinterfaces_BINARY_DIR}/src>")


set_target_properties(KF5KdepimDBusInterfaces PROPERTIES
    VERSION ${KDEPIMDBUSINTERFACES_VERSION_STRING}
    SOVERSION ${KDEPIMDBUSINTERFACES_SOVERSION}
    EXPORT_NAME KdepimDBusInterfaces
)

install(TARGETS
    KF5KdepimDBusInterfaces
    EXPORT KF5KdepimDBusInterfacesTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(kdepimdbusinterfaces_Camelcase_HEADERS
  HEADER_NAMES
  ReminderClient
  UriHandler
  REQUIRED_HEADERS kdepimdbusinterfaces_HEADERS
  PREFIX KdepimDBusInterfaces
)

ecm_generate_pri_file(BASE_NAME KdepimDBusInterfaces
    LIB_NAME KF5KdepimDBusInterfaces
    DEPS "AkonadiCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KdepimDBusInterfaces
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${kdepimdbusinterfaces_SOURCE_DIR})
install(FILES
    ${kdepimdbusinterfaces_Camelcase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KdepimDBusInterfaces
    COMPONENT Devel
)

install(FILES
    ${kdepimdbusinterfaces_HEADERS}
    ${kdepimdbusinterfaces_Camelcase_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kdepimdbusinterfaces_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/kdepimdbusinterfaces
    COMPONENT Devel
)

install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()

