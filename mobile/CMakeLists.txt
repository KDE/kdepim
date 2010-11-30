add_definitions( -DKDEPIM_MOBILE_UI )

add_subdirectory(lib)
add_subdirectory(pics)

include_directories(lib)

add_subdirectory(calendar)
add_subdirectory(contacts)
add_subdirectory(mail)
add_subdirectory(tasks)
add_subdirectory(notes)

add_subdirectory(licenses)
