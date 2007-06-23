include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs)

set(service_SRCS service.cpp)

kde4_add_ui_files(service_SRCS testserviceview.ui)

qt4_add_dbus_interface(service_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../libs/org.kde.Solid.Networking.Service.xml serviceinterface)

kde4_automoc(${service_SRCS})

kde4_add_executable(service ${service_SRCS})

target_link_libraries(service ${KDE4_KIO_LIBS})

########### next target ###############

set(client_SRCS client.cpp)

kde4_add_ui_files(client_SRCS testclientview.ui)

qt4_add_dbus_interface(client_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../libs/org.kde.Solid.Networking.Client.xml clientinterface)

kde4_automoc(${client_SRCS})

kde4_add_executable(client ${client_SRCS})

target_link_libraries(client kconnectionmanager ${KDE4_KIO_LIBS})
