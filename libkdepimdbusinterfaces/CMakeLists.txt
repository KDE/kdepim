set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_definitions(${QT_QTDBUS_DEFINITIONS})

########### next target ###############

set(kdepimdbusinterfaces_LIB_SRCS
    urihandler.cpp
    reminderclient.cpp
)

qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/korgac/org.kde.korganizer.KOrgac.xml)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)
qt4_add_dbus_interfaces( kdepimdbusinterfaces_LIB_SRCS ${CMAKE_SOURCE_DIR}/knode/org.kde.knode.xml)

kde4_add_library(kdepimdbusinterfaces SHARED ${kdepimdbusinterfaces_LIB_SRCS})

add_dependencies(kdepimdbusinterfaces kmail_xml)

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

