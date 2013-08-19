set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(
  ${CMAKE_SOURCE_DIR}/kleopatra
  ${CMAKE_BINARY_DIR}/kleopatra
  ${CMAKE_SOURCE_DIR}/libkleo
  ${Boost_INCLUDE_DIR}
  ${GPGME_INCLUDES}
)

########### next target ###############

if(PORT_ME_AWAY_FROM_EVENT_LOOPS)
  set(test_keylistmodels_SRCS
    test_keylistmodels.cpp
    ../models/keylistmodel.cpp
    ../models/keylistsortfilterproxymodel.cpp
    ../utils/formatting.cpp
  )
  if(KLEO_MODEL_TEST)
    set(test_keylistmodels_SRCS
      ${test_keylistmodels_SRCS}
      ../models/modeltest.cpp
    )
  endif()

  kde4_add_executable(test_keylistmodels TEST ${test_keylistmodels_SRCS})

  target_link_libraries(test_keylistmodels kleo ${KDE4_KDECORE_LIBS})

########### next target ###############

  set(test_useridlistmodels_SRCS
    test_useridlistmodels.cpp
    ../models/useridlistmodel.cpp
    ../utils/formatting.cpp
  )
  if(KLEO_MODEL_TEST)
    set(test_useridlistmodels_SRCS
      ${test_useridlistmodels_SRCS}
      ../models/modeltest.cpp
    )
  endif()

  kde4_add_executable(test_useridlistmodels TEST ${test_useridlistmodels_SRCS})

  target_link_libraries(test_useridlistmodels kleo ${KDE4_KDECORE_LIBS})

endif()

########### next target ###############

set(test_verify_SRCS test_verify.cpp)

add_definitions(-DKLEO_TEST_GNUPGHOME=\\"${CMAKE_CURRENT_SOURCE_DIR}/gnupg_home\\")
add_definitions(-DKLEO_TEST_DATADIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")

kde4_add_unit_test(test_verify TESTNAME kleo-verifytest ${test_verify_SRCS})

target_link_libraries(test_verify
  kleo
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${QGPGME_LIBRARIES}
)

########### next target ###############

if(USABLE_ASSUAN_FOUND)

  # this doesn't yet work on Windows

  add_definitions(-DGPG_ERR_SOURCE_DEFAULT=GPG_ERR_SOURCE_USER_2)

  set(test_uiserver_SRCS test_uiserver.cpp ../utils/wsastarter.cpp ../utils/hex.cpp)

#FIXME: omitting TEST makes test_uiserver print output again on a Win32 console;
# find a better fix for this issue
  if(WIN32)
    kde4_add_executable(test_uiserver ${test_uiserver_SRCS})
  else()
    kde4_add_executable(test_uiserver TEST ${test_uiserver_SRCS})
  endif()

  if(ASSUAN2_FOUND)
    target_link_libraries(test_uiserver
      kleo
      ${KDE4_KDECORE_LIBS}
      ${ASSUAN2_LIBRARIES}
    )
  else()
    target_link_libraries(test_uiserver
      kleo
      ${KDE4_KDECORE_LIBS}
      ${ASSUAN_LIBRARIES}
    )
  endif()

  if(WIN32)
    target_link_libraries(test_uiserver
      ${ASSUAN_VANILLA_LIBRARIES}
      ${QGPGME_LIBRARIES}
      ws2_32
    )
  else()
    target_link_libraries(test_uiserver
      ${ASSUAN_PTHREAD_LIBRARIES}
      ${QGPGME_LIBRARIES}
  )
  endif()

  if(KDEPIM_ONLY_KLEO)
    install(TARGETS test_uiserver ${INSTALL_TARGETS_DEFAULT_ARGS})
  endif()

endif()

