include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
)

set( pimcommon_imapaclattributetest_source imapaclattributetest.cpp ../imapaclattribute.cpp )
kde4_add_unit_test( imapaclattributetest ${pimcommon_imapaclattributetest_source})
target_link_libraries( imapaclattributetest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS}  ${KDEPIMLIBS_KIMAP_LIBS})
