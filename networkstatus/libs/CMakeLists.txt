include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../includes)

set(libkconnectionmanager_SRCS kconnectionmanager.cpp kconnectionmanager_p.cpp networkstatusindicator.cpp )

qt4_add_dbus_interface(libkconnectionmanager_SRCS org.kde.Solid.Networking.Client.xml
    clientinterface)

kde4_automoc(${libkconnectionmanager_SRCS})

kde4_add_library(kconnectionmanager SHARED ${libkconnectionmanager_SRCS})

target_link_libraries(kconnectionmanager ${KDE4_KIO_LIBS})

install(TARGETS kconnectionmanager DESTINATION ${LIB_INSTALL_DIR})

install(FILES kconnectionmanager.h
              networkstatuscommon.h
              networkstatusindicator.h
        DESTINATION ${INCLUDE_INSTALL_DIR})
