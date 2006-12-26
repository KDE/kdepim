
include_directories( ${CMAKE_SOURCE_DIR}/libkleo  )


########### next target ###############

set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp identitycombo.cpp)

kde4_automoc(${kpimidentities_LIB_SRCS})

qt4_add_dbus_adaptor(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIM::IdentityManager )
qt4_add_dbus_interfaces(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml)

kde4_add_library(kpimidentities SHARED ${kpimidentities_LIB_SRCS})

target_link_libraries(kpimidentities  ${KDE4_KDEUI_LIBS} kleopatra kdepim )

set_target_properties(kpimidentities PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kpimidentities  DESTINATION ${LIB_INSTALL_DIR})

install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_DIR})
