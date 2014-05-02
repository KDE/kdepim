kde_enable_exceptions()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


find_package(KF5AkonadiContact ${KF5_VERSION} CONFIG REQUIRED)


########### next target ###############

set(kdepimdbusinterfaces_LIB_SRCS
    urihandler.cpp
    reminderclient.cpp
)

qt4_generate_dbus_interface(${CMAKE_SOURCE_DIR}/kmail/kmkernel.h org.kde.kmail.kmail.xml OPTIONS -a)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korgac/org.kde.korganizer.KOrgac.xml)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)
qt5_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.kmail.xml)

add_library(kdepimdbusinterfaces ${LIBRARY_TYPE} ${kdepimdbusinterfaces_LIB_SRCS})

add_dependencies(kdepimdbusinterfaces kmail_xml)

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(kdepimdbusinterfaces
    KF5::AkonadiCore
    KF5::AkonadiContact 
    KF5::KIOCore
)

set_target_properties(kdepimdbusinterfaces PROPERTIES VERSION ${KF5_VERSION} SOVERSION ${KF5_VERSION})
install(TARGETS kdepimdbusinterfaces ${INSTALL_TARGETS_DEFAULT_ARGS})

