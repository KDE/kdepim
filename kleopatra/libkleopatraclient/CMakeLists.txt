set(libkleopatraclient_version "1.3.0")
set(libkleopatraclient_soversion "1")

add_subdirectory(core)
add_subdirectory(gui)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
