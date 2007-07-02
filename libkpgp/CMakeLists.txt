
add_subdirectory( pics ) 

include_directories(${CMAKE_SOURCE_DIR}/libkdepim   )


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

kde4_automoc(${kpgp_LIB_SRCS})

kde4_add_library(kpgp-gpl SHARED ${kpgp_LIB_SRCS})

target_link_libraries(kpgp-gpl  ${KDE4_KDE3SUPPORT_LIBS} )

set_target_properties(kpgp-gpl PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kpgp-gpl  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kpgp.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS kpgp-3.1-upgrade-address-data.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

install(FILES
        kpgp.h
        kpgpbase.h
        kpgpblock.h
        kpgpkey.h
        kpgpui.h
        libkpgp_export.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kpgp )

