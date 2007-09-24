set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} )

########### next target ###############

set(test_keylistmodels_SRCS test_keylistmodels.cpp ../models/keylistmodel.cpp ../models/keylistsortfilterproxymodel.cpp ../utils/formatting.cpp )

kde4_add_executable(test_keylistmodels TEST ${test_keylistmodels_SRCS})

target_link_libraries(test_keylistmodels kleo gpgme++ ${KDE4_KDECORE_LIBS} )

