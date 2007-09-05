include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${GPGME_INCLUDES} )

########### next target ###############

set(test_keylistmodels_SRCS test_keylistmodels.cpp ../models/keylistmodel.cpp ../models/keylistsortfilterproxymodel.cpp ../utils/formatting.cpp )

kde4_add_test_executable(test_keylistmodels ${test_keylistmodels_SRCS})

target_link_libraries(test_keylistmodels kleo ${KDE4_KDECORE_LIBS} )

########### next target ###############

if ( USABLE_ASSUAN_FOUND AND NOT WIN32 )

  # this doesn't yet work on Windows

  set(test_uiserver_SRCS test_uiserver.cpp ../uiserver/kleo-assuan.cpp )

  kde4_add_test_executable(test_uiserver ${test_uiserver_SRCS})

  target_link_libraries(test_uiserver ${KDE4_KDECORE_LIBS} ${ASSUAN_PTHREAD_LIBRARIES} )

endif( USABLE_ASSUAN_FOUND AND NOT WIN32 )

