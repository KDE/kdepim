set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ../ )

set( conflictresolvertest_SRCS
     conflictresolvertest.cpp )

kde4_add_unit_test(conflictresolvertest  ${conflictresolvertest_SRCS} )

target_link_libraries(
  conflictresolvertest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  incidenceeditorsng
  akonadi-kcal_next
  kdepimdbusinterfaces
)
