project(kontactinterfaces)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5601)

set(kontactinterfaces_LIB_SRCS
    core.cpp
    plugin.cpp
    summary.cpp
    uniqueapphandler.cpp)

kde4_add_library(kontactinterfaces SHARED ${kontactinterfaces_LIB_SRCS})

target_link_libraries(kontactinterfaces ${KDE4_KPARTS_LIBS})

set_target_properties(kontactinterfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

########### install files ###############

install(TARGETS kontactinterfaces DESTINATION ${LIB_INSTALL_DIR})

install(FILES kontactplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
