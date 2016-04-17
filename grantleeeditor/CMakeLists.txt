project(grantleeditor)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory(headerthemeeditor)
add_subdirectory(contactthemeeditor)
add_subdirectory(grantleethemeeditor)
add_subdirectory(contactprintthemeeditor)


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
