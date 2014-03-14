include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
)
# convenience macro to add qtest unit tests
macro(add_plaintext_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME pimcommon-${_name} ${_test})
  target_link_libraries( ${_name}
    pimcommon
    ${QGPGME_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
  )
endmacro ()

add_plaintext_unittest( textgotolinewidgettest.cpp )

