project( sieveeditor )

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories(
  ${CMAKE_SOURCE_DIR}/libksieve/
)


set(sieveeditor_SRCS
  main.cpp
)


kde4_add_executable(sieveeditor ${sieveeditor_SRCS} )
TARGET_LINK_LIBRARIES(sieveeditor 
                       ${KDE4_KDEUI_LIBS} 
                       ${KDE4_KNEWSTUFF3_LIBS} 
                       pimcommon 
                       ksieveui
)

install(TARGETS sieveeditor ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  sieveeditorui.rc
  DESTINATION  ${DATA_INSTALL_DIR}/sieveeditor)

install(PROGRAMS sieveeditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
