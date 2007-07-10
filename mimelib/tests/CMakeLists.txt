
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(test_boyermor_SRCS test_boyermor.cpp )



kde4_add_test_executable(test_boyermor ${test_boyermor_SRCS})

target_link_libraries(test_boyermor  ${KDE4_KDECORE_LIBS} mimelib )

