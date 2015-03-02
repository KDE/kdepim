set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
)

set( libkdepim_baloocompletionemailtest_SRCS baloocompletionemailtest.cpp ../addresslineedit/baloocompletionemail.cpp)
kde4_add_unit_test( libkdepim_baloocompletionemailtest ${libkdepim_baloocompletionemailtest_SRCS})
target_link_libraries( libkdepim_baloocompletionemailtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KPIMUTILS_LIBS})


set( libkdepim_recentaddresswidgettest_SRCS recentaddresswidgettest.cpp ../recentaddress/recentaddresswidget.cpp ../recentaddress/recentaddresses.cpp)
kde4_add_unit_test( libkdepim_recentaddresswidgettest ${libkdepim_recentaddresswidgettest_SRCS})
target_link_libraries( libkdepim_recentaddresswidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KPIMUTILS_LIBS})



set( libkdepim_completionconfiguredialogtest_SRCS completionconfiguredialogtest.cpp )
kde4_add_unit_test( libkdepim_completionconfiguredialogtest ${libkdepim_completionconfiguredialogtest_SRCS})
target_link_libraries( libkdepim_completionconfiguredialogtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdepim)

set( libkdepim_completionorderwidgettest_SRCS completionorderwidgettest.cpp )
kde4_add_unit_test( libkdepim_completionorderwidgettest ${libkdepim_completionorderwidgettest_SRCS})
target_link_libraries( libkdepim_completionorderwidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdepim)
