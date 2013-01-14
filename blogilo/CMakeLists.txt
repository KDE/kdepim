project(blogilo)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
   # if we get here, blogilo is built independent from the rest of kdepim
   include(${CMAKE_SOURCE_DIR}/blogilo-separate.cmake)
endif()

add_subdirectory( src )
add_subdirectory( icons )
