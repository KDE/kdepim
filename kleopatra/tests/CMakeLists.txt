include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} )

########### next target ###############

set(test_keylistmodels_SRCS test_keylistmodels.cpp ../models/keylistmodel.cpp )

kde4_add_test_executable(test_keylistmodels ${test_keylistmodels_SRCS})

target_link_libraries(test_keylistmodels kleo gpgme++ ${KDE4_KDECORE_LIBS} )

