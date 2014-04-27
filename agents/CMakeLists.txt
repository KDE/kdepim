add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

add_subdirectory(sendlateragent)
add_subdirectory(archivemailagent)
add_subdirectory(mailfilteragent)
#REACTIVATE
#add_subdirectory(notesagent)
install(FILES folderarchiveagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
