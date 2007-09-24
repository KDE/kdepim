set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(test_boyermor_SRCS test_boyermor.cpp )



kde4_add_executable(test_boyermor TEST ${test_boyermor_SRCS})

target_link_libraries(test_boyermor  ${KDE4_KDECORE_LIBS} mimelib )

