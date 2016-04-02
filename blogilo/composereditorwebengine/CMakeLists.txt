project(composereditorwebengine)

add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

add_subdirectory(src)
if (BUILD_TESTING)
   add_subdirectory(tests)
   add_subdirectory(autotests)
endif()
