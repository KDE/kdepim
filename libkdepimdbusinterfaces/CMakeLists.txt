set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(${QT_QTDBUS_DEFINITIONS})

########### next target ###############

set(kdepimdbusinterfaces_LIB_SRCS
    urihandler.cpp
    reminderclient.cpp
)

qt4_generate_dbus_interface(${CMAKE_SOURCE_DIR}/kmail/kmkernel.h org.kde.kmail.kmail.xml OPTIONS -a)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korgac/org.kde.korganizer.KOrgac.xml)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kmail.kmail.xml)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/knode/org.kde.knode.xml)

kde4_add_library(kdepimdbusinterfaces ${LIBRARY_TYPE} ${kdepimdbusinterfaces_LIB_SRCS})

add_dependencies(kdepimdbusinterfaces kmail_xml)

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(kdepimdbusinterfaces
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} 
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

set_target_properties(kdepimdbusinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepimdbusinterfaces ${INSTALL_TARGETS_DEFAULT_ARGS})

