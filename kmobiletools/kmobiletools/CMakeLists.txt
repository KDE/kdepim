
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
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

########### next target ###############

set(kmobiletools_SRCS main.cpp kmobiletools.cpp )

kde4_automoc(${kmobiletools_SRCS})

kde4_add_executable(kmobiletools_bin ${kmobiletools_SRCS})
set_target_properties(kmobiletools_bin PROPERTIES OUTPUT_NAME kmobiletools)

target_link_libraries(kmobiletools_bin ${KDE4_KPARTS_LIBS} kmobiletools )

install(TARGETS kmobiletools_bin DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  kmobiletools.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES  kmobiletoolsui.rc DESTINATION ${DATA_INSTALL_DIR}/kmobiletools )

