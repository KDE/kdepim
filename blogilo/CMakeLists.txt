project(blogilo)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
   # if we get here, blogilo is built independent from the rest of kdepim
   set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
   include(${CMAKE_SOURCE_DIR}/blogilo-separate.cmake)
else("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
   set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory( cmake )

find_package(QJson)
if(QJSON_FOUND)
add_subdirectory( src )
add_subdirectory( icons )
endif()
