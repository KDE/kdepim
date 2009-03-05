set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_BINARY_DIR}/kleopatra ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${GPGME_INCLUDES} )

########### next target ###############

if ( PORT_ME_AWAY_FROM_EVENT_LOOPS )
set(test_keylistmodels_SRCS test_keylistmodels.cpp ../models/keylistmodel.cpp ../models/keylistsortfilterproxymodel.cpp ../utils/formatting.cpp ../utils/exception.cpp )
if ( KLEO_MODEL_TEST )
   set(test_keylistmodels_SRCS ${test_keylistmodels_SRCS} ../models/modeltest.cpp )
endif ( KLEO_MODEL_TEST )

kde4_add_executable(test_keylistmodels TEST ${test_keylistmodels_SRCS})

target_link_libraries(test_keylistmodels kleo ${KDE4_KDECORE_LIBS} )

########### next target ###############

set(test_useridlistmodels_SRCS test_useridlistmodels.cpp ../models/useridlistmodel.cpp ../utils/formatting.cpp ../utils/exception.cpp )
if ( KLEO_MODEL_TEST )
   set(test_useridlistmodels_SRCS ${test_useridlistmodels_SRCS} ../models/modeltest.cpp )
endif ( KLEO_MODEL_TEST )

kde4_add_executable(test_useridlistmodels TEST ${test_useridlistmodels_SRCS})

target_link_libraries(test_useridlistmodels kleo ${KDE4_KDECORE_LIBS} )

########### next target ###############

#set(test_verificationresultdialog_SRCS test_verificationresultdialog.cpp ../uiserver/signaturedisplaywidget.cpp ../utils/formatting.cpp ../certificateinfowidgetimpl.cpp ../uiserver/resultdisplaywidget.cpp)
#kde4_add_ui_files( test_verificationresultdialog_SRCS ../certificateinfowidget.ui)
#kde4_add_executable(test_verificationresultdialog TEST ${test_verificationresultdialog_SRCS})
#
#target_link_libraries(test_verificationresultdialog kleo ${KDE4_KDECORE_LIBS} ${QT_QT3SUPPORT_LIBRARY})

endif ( PORT_ME_AWAY_FROM_EVENT_LOOPS )

########### next target ###############

set(test_verify_SRCS test_verify.cpp)
add_definitions( -DKLEO_TEST_GNUPGHOME=\\"${CMAKE_CURRENT_SOURCE_DIR}/gnupg_home\\" )
add_definitions( -DKLEO_TEST_DATADIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )
kde4_add_unit_test(test_verify TESTNAME kleo-verifytest ${test_verify_SRCS})
target_link_libraries(test_verify kleo ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${QGPGME_LIBRARIES})

########### next target ###############

if ( USABLE_ASSUAN_FOUND  )

  # this doesn't yet work on Windows

  add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_2 )

  set(test_uiserver_SRCS test_uiserver.cpp ../utils/exception.cpp ../utils/wsastarter.cpp ../utils/hex.cpp )

#FIXME: omitting TEST makes test_uiserver print output again on a Win32 console;
# find a better fix for this issue
  if(WIN32)
    kde4_add_executable(test_uiserver ${test_uiserver_SRCS})
  else(WIN32)
    kde4_add_executable(test_uiserver TEST ${test_uiserver_SRCS})
  endif(WIN32)

  target_link_libraries(test_uiserver ${KDE4_KDECORE_LIBS} )
  if(WIN32)
	 target_link_libraries(test_uiserver ${ASSUAN_VANILLA_LIBRARIES} ${QGPGME_LIBRARIES} ws2_32)
  else(WIN32)
	 target_link_libraries(test_uiserver ${ASSUAN_PTHREAD_LIBRARIES} ${QGPGME_LIBRARIES} )
  endif(WIN32)
  if (KDEPIM_ONLY_KLEO)
     install( TARGETS test_uiserver ${INSTALL_TARGETS_DEFAULT_ARGS})
  endif(KDEPIM_ONLY_KLEO)
endif( USABLE_ASSUAN_FOUND  )

