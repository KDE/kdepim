configure_file (config-libkpimidentities.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libkpimidentities.h)

if(GPGME_FOUND)
   include_directories( ${CMAKE_SOURCE_DIR}/libkleo  )
endif(GPGME_FOUND)


########### next target ###############

set(kpimidentities_LIB_SRCS identitymanager.cpp identity.cpp identitycombo.cpp)

kde4_automoc(${kpimidentities_LIB_SRCS})

qt4_add_dbus_adaptor(kpimidentities_LIB_SRCS org.kde.pim.IdentityManager.xml identitymanager.h KPIM::IdentityManager )

kde4_add_library(kpimidentities SHARED ${kpimidentities_LIB_SRCS})

if(GPGME_FOUND)
   target_link_libraries(kpimidentities  ${KDE4_KDEUI_LIBS} kleopatra kdepim )
else(GPGME_FOUND)
   target_link_libraries(kpimidentities  ${KDE4_KDEUI_LIBS} kdepim )
endif(GPGME_FOUND)

set_target_properties(kpimidentities PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpimidentities  DESTINATION ${LIB_INSTALL_DIR})

install(FILES org.kde.pim.IdentityManager.xml DESTINATION ${DBUS_INTERFACES_DIR})
