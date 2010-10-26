include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

################ History #############################
set( historytests_SRCS historytest.cpp  )

kde4_add_unit_test( historytests
  TESTNAME historytests
  ${historytests_SRCS}
)

target_link_libraries( historytests
  calendarsupport
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
)

############## IncidenceChanger ########################
set( incidencechangertests_SRCS incidencechangertest.cpp  )

kde4_add_unit_test( incidencechangertests
  TESTNAME incidencechangertests
  ${incidencechangertests_SRCS}
)

target_link_libraries( incidencechangertests
  calendarsupport
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
)
