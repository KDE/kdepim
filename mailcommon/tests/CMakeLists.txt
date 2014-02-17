set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(
  BEFORE
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${CMAKE_BINARY_DIR}/mailcommon
)

MACRO(MAILCOMMON_ADD_UNITTEST _source )
  set(_test ${_source} ${ARGN})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME mailcommon-${_name} ${_test})
  target_link_libraries(
    ${_name}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    messagecore
    messageviewer
    #kmailprivate
  )
ENDMACRO(MAILCOMMON_ADD_UNITTEST)

mailcommon_add_unittest( searchpatterntest.cpp )
