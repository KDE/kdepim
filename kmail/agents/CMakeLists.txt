add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

add_subdirectory(sendlateragent)
add_subdirectory(archivemailagent)
add_subdirectory(mailfilteragent)
add_subdirectory(followupreminderagent)
