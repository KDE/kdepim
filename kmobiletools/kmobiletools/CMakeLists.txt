
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/libkmobiletools
#   ${CMAKE_CURRENT_SOURCE_DIR}/libkmobiletools
#   ${CMAKE_CURRENT_BINARY_DIR}/libkmobiletools
   ${CMAKE_CURRENT_SOURCE_DIR}/mainpart
   ${CMAKE_CURRENT_SOURCE_DIR}/engines/
#    ${CMAKE_CURRENT_BINARY_DIR}/engines/libkmobiletoolsengineui
)

add_subdirectory( libkmobiletools )
add_subdirectory( about )
add_subdirectory( mainpart )
add_subdirectory( engines )
add_subdirectory( tests )

########### next target ###############

set(kmobiletools_SRCS main.cpp kmobiletools.cpp kmobiletools.h )

# todo: move to Oxygen icon
kde4_add_app_icon(kmobiletools_SRCS "libkmobiletools/icons/hi*-app-kmobiletools.png")

kde4_add_executable(kmobiletools_bin ${kmobiletools_SRCS})
set_target_properties(kmobiletools_bin PROPERTIES OUTPUT_NAME kmobiletools)

target_link_libraries(kmobiletools_bin ${KDE4_KPARTS_LIBS} kmobiletoolslib )

install(TARGETS kmobiletools_bin ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES  kmobiletools.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES  kmobiletoolsui.rc DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )

