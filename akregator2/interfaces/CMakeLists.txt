########### next target ###############

include_directories( ${Boost_INCLUDE_DIR} )

set(akregator2interfaces_LIB_SRCS
   command.cpp
   plugin.cpp
)

kde4_add_kcfg_files(akregator2interfaces_LIB_SRCS akregator2config.kcfgc)

kde4_add_library(akregator2interfaces SHARED ${akregator2interfaces_LIB_SRCS})

target_link_libraries(akregator2interfaces kdepim ${KDE4_KDECORE_LIBS})

set_target_properties(akregator2interfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS akregator2interfaces ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES akregator2.kcfg DESTINATION ${KCFG_INSTALL_DIR})

