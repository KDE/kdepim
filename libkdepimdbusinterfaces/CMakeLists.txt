project(libkdepimdbusinterfaces)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


########### next target ###############

set(kdepimdbusinterfaces_LIB_SRCS
    urihandler.cpp
    reminderclient.cpp
    kdepimdbusinterface_debug.cpp
)

ecm_qt_declare_logging_category(kdepimdbusinterfaces_LIB_SRCS HEADER kdepimdbusinterface_debug.h IDENTIFIER KDEPIMDBUSINTERFACE_LOG CATEGORY_NAME log_kdepimdbusinterface)


qt5_generate_dbus_interface(${CMAKE_SOURCE_DIR}/kmail/kmkernel.h org.kde.kmail.kmail.xml OPTIONS -a)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korgac/org.kde.korganizer.KOrgac.xml)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/data/org.kde.korganizer.Korganizer.xml)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.kmail.xml)

add_library(kdepimdbusinterfaces ${kdepimdbusinterfaces_LIB_SRCS})
generate_export_header(kdepimdbusinterfaces BASE_NAME kdepimdbusinterfaces)

add_dependencies(kdepimdbusinterfaces kmail_xml)

target_link_libraries(kdepimdbusinterfaces
PUBLIC
    KF5::AkonadiCore
PRIVATE
    KF5::AkonadiContact
    KF5::KIOWidgets
    Qt5::DBus
)

set_target_properties(kdepimdbusinterfaces
    PROPERTIES VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
)
install(TARGETS kdepimdbusinterfaces ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

target_include_directories(kdepimdbusinterfaces PUBLIC "$<BUILD_INTERFACE:${libkdepimdbusinterfaces_SOURCE_DIR};${libkdepimdbusinterfaces_BINARY_DIR}>")
