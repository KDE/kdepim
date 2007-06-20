include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/libkmobiletools
   )
set(testlibkmobiletools_SRCS testlibkmobiletools.cpp )

kde4_automoc(${testlibkmobiletools_SRCS})

kde4_add_executable(testlibkmobiletools ${testlibkmobiletools_SRCS})
set_target_properties(testlibkmobiletools PROPERTIES OUTPUT_NAME testlibkmobiletools)

target_link_libraries(testlibkmobiletools kmobiletools )
