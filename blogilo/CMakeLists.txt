project(blogilo)

cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_subdirectory( src )
add_subdirectory( icons )
