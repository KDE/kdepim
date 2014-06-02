add_definitions( -DKDEPIM_MOBILE_UI )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


add_subdirectory(lib)
add_subdirectory(pics)

include_directories(lib)

add_subdirectory(calendar)
add_subdirectory(contacts)
add_subdirectory(mail)
add_subdirectory(tasks)
add_subdirectory(notes)

add_subdirectory(api)

add_subdirectory(licenses)
