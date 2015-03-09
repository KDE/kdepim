include_directories(
  ${CMAKE_SOURCE_DIR}/libksieve/
)

set( libksieveui_sieveeditormenubartest_source sieveeditormenubartest.cpp ../sieveeditormenubar.cpp )
kde4_add_unit_test( sieveeditormenubartest ${libksieveui_sieveeditormenubartest_source})
target_link_libraries( sieveeditormenubartest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} )



set( libksieveui_sievetextedittest_source sievetextedittest.cpp )
kde4_add_unit_test( sievetextedittest ${libksieveui_sievetextedittest_source})
target_link_libraries( sievetextedittest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ksieveui pimcommon)

