project(networkstatus)

add_subdirectory(libs)
add_subdirectory(tests)

########### next target ###############

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(kded_networkstatus_PART_SRCS networkstatus.cpp network.cpp)

qt4_add_dbus_adaptor(kded_networkstatus_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libs/org.kde.Solid.Networking.Service.xml
        networkstatus.h NetworkStatusModule)

qt4_add_dbus_adaptor(kded_networkstatus_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libs/org.kde.Solid.Networking.Client.xml
        networkstatus.h NetworkStatusModule)

kde4_automoc(${kded_networkstatus_PART_SRCS})

kde4_add_plugin(kded_networkstatus ${kded_networkstatus_PART_SRCS})

target_link_libraries(kded_networkstatus  ${KDE4_KIO_LIBS})

install(TARGETS kded_networkstatus  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkstatus.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)

