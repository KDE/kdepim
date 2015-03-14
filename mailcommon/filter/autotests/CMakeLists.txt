include_directories(
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${CMAKE_SOURCE_DIR}/mailcommon/filter
)

set( mailcommon_itemcontexttest_source itemcontexttest.cpp ../itemcontext.cpp )
kde4_add_unit_test( itemcontexttest ${mailcommon_itemcontexttest_source})
target_link_libraries( itemcontexttest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})

set( mailcommon_filteractionpipethroughtest_source
   actionpipethrough.cpp
   ../filteractions/filteractionpipethrough.cpp
   ../filteractions/filteractionwithcommand.cpp
   ../filteractions/filteractionwithurl.cpp
)
kde4_add_unit_test( filteractionpipethroughtest ${mailcommon_filteractionpipethroughtest_source} )
target_link_libraries( filteractionpipethroughtest
   kdepim
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)
