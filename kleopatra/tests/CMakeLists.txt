include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${GPGME_INCLUDES} )

########### next target ###############

set(test_keylistmodels_SRCS test_keylistmodels.cpp ../models/keylistmodel.cpp ../models/keylistsortfilterproxymodel.cpp ../utils/formatting.cpp )

kde4_add_test_executable(test_keylistmodels ${test_keylistmodels_SRCS})

target_link_libraries(test_keylistmodels kleo ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_verificationresultdialog_SRCS test_verificationresultdialog.cpp ../uiserver/signaturedisplaywidget.cpp )

kde4_add_test_executable(test_verificationresultdialog ${test_verificationresultdialog_SRCS})

target_link_libraries(test_verificationresultdialog kleo ${KDE4_KDECORE_LIBS} )

########### next target ###############

if ( USABLE_ASSUAN_FOUND  )

  # this doesn't yet work on Windows

  add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_2 )

  set(test_uiserver_SRCS test_uiserver.cpp ../uiserver/kleo-assuan.cpp )

  kde4_add_test_executable(test_uiserver ${test_uiserver_SRCS})

  target_link_libraries(test_uiserver ${KDE4_KDECORE_LIBS} )
  if(WIN32)
	 target_link_libraries(test_uiserver ${ASSUAN_VANILLA_LIBRARIES} Ws2_32)
  else(WIN32)
	 target_link_libraries(test_uiserver ${ASSUAN_PTHREAD_LIBRARIES} ) 
  endif(WIN32)
endif( USABLE_ASSUAN_FOUND  )

