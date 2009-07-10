
add_subdirectory( pics )

include_directories( ${CMAKE_SOURCE_DIR}/libkdepim )

INCLUDE(CheckIncludeFiles)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
configure_file(config-libkpgp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libkpgp.h)

########### next target ###############

set(kpgp_LIB_SRCS
   kpgpblock.cpp
   kpgpui.cpp
   kpgpkey.cpp
   kpgp.cpp
   kpgpbase.cpp
   kpgpbaseG.cpp
   kpgpbase2.cpp
   kpgpbase5.cpp
   kpgpbase6.cpp )


kde4_add_library(kpgp SHARED ${kpgp_LIB_SRCS})

target_link_libraries(kpgp  ${KDE4_KDE3SUPPORT_LIBS})

set_target_properties(kpgp PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kpgp ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES kpgp.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS kpgp-3.1-upgrade-address-data.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

install(FILES
        kpgp.h
        kpgpbase.h
        kpgpblock.h
        kpgpkey.h
        kpgpui.h
        libkpgp_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kpgp COMPONENT Devel )

