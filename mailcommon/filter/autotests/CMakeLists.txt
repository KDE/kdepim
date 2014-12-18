include_directories(
  ${CMAKE_SOURCE_DIR}/mailcommon
)

set( mailcommon_itemcontexttest_source itemcontexttest.cpp ../itemcontext.cpp )
kde4_add_unit_test( itemcontexttest ${mailcommon_itemcontexttest_source})
target_link_libraries( itemcontexttest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})
