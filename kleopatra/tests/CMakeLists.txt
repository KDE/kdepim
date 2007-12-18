set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_BINARY_DIR}/kleopatra ${CMAKE_SOURCE_DIR}/libkleo ${Boost_INCLUDE_DIR} ${GPGME_INCLUDES} )

########### next target ###############

set(test_keylistmodels_SRCS test_keylistmodels.cpp ../models/keylistmodel.cpp ../models/keylistsortfilterproxymodel.cpp ../utils/formatting.cpp )

kde4_add_executable(test_keylistmodels TEST ${test_keylistmodels_SRCS})

target_link_libraries(test_keylistmodels kleo ${KDE4_KDECORE_LIBS} )

########### next target ###############

#set(test_verificationresultdialog_SRCS test_verificationresultdialog.cpp ../uiserver/signaturedisplaywidget.cpp ../utils/formatting.cpp ../certificateinfowidgetimpl.cpp ../uiserver/resultdisplaywidget.cpp)
#kde4_add_ui_files( test_verificationresultdialog_SRCS ../certificateinfowidget.ui)
#kde4_add_executable(test_verificationresultdialog TEST ${test_verificationresultdialog_SRCS})
#
#target_link_libraries(test_verificationresultdialog kleo ${KDE4_KDECORE_LIBS} ${QT_QT3SUPPORT_LIBRARY})


########### next target ###############

kde4_add_executable( test_signencryptwizard TEST
  test_signencryptwizard.cpp
  ../uiserver/certificateresolver.cpp
  ../uiserver/signencryptwizard.cpp 
  ../uiserver/resolverecipientspage.cpp
  ../uiserver/keyselectiondialog.cpp
  ../uiserver/kleo-assuan.cpp
  ../uiserver/objectspage.cpp
  ../uiserver/wizardresultpage.cpp
  ../uiserver/resultdisplaywidget.cpp
  ../uiserver/scrollarea.cpp
  ../uiserver/signerresolvepage.cpp
  ../uiserver/wizardpage.cpp
  ../uiserver/wizard.cpp
  ../uiserver/signingcertificateselectiondialog.cpp
  ../certificateinfowidgetimpl.cpp
  ../utils/formatting.cpp
  ../models/keycache.cpp 
  ../models/keylistmodel.cpp )
target_link_libraries( test_signencryptwizard kleo ${KDE4_KMIME_LIBRARY} ${KDE4_KDEUI_LIBS} ${QGPGME_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY} )

########### next target ###############

set(test_verify_SRCS test_verify.cpp)
add_definitions( -DKLEO_TEST_GNUPGHOME=\\"${CMAKE_CURRENT_SOURCE_DIR}/gnupg_home\\" )
add_definitions( -DKLEO_TEST_DATADIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )
kde4_add_unit_test(test_verify TESTNAME kleo-verifytest ${test_verify_SRCS})
target_link_libraries(test_verify kleo ${QT_QTTEST_LIBRARY})

########### next target ###############

if ( USABLE_ASSUAN_FOUND  )

  # this doesn't yet work on Windows

  add_definitions( -DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_2 )

  set(test_uiserver_SRCS test_uiserver.cpp ../uiserver/kleo-assuan.cpp ../utils/wsastarter.cpp ../uiserver/hex.cpp )

#FIXME: omitting TEST makes test_uiserver print output again on a Win32 console;
# find a better fix for this issue
  if(WIN32)
    kde4_add_executable(test_uiserver ${test_uiserver_SRCS})
  else(WIN32)
    kde4_add_executable(test_uiserver TEST ${test_uiserver_SRCS})
  endif(WIN32)

  target_link_libraries(test_uiserver ${KDE4_KDECORE_LIBS} )
  if(WIN32)
	 target_link_libraries(test_uiserver ${ASSUAN_VANILLA_LIBRARIES} ${QGPGME_LIBRARIES} Ws2_32)
  else(WIN32)
	 target_link_libraries(test_uiserver ${ASSUAN_PTHREAD_LIBRARIES} ${QGPGME_LIBRARIES} )
  endif(WIN32)
  if (ONLY_KLEO)
     install( TARGETS test_uiserver DESTINATION ${BIN_INSTALL_DIR})
  endif(ONLY_KLEO)
endif( USABLE_ASSUAN_FOUND  )

