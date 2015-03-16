include_directories(
  ${CMAKE_SOURCE_DIR}/mailcommon
  ${CMAKE_SOURCE_DIR}/mailcommon/filter
)

set( mailcommon_itemcontexttest_source itemcontexttest.cpp ../itemcontext.cpp )
kde4_add_unit_test( itemcontexttest ${mailcommon_itemcontexttest_source})
target_link_libraries( itemcontexttest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS})

set( mailcommon_filteractionpipethroughtest_source
   filteractionpipethroughtest.cpp
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

set(mailcommon_filteractionheadertest_SRCS filteractionaddheadertest.cpp
                                           ../filteractions/filteractionaddheader.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionaddheadertest ${mailcommon_filteractionheadertest_SRCS} )
target_link_libraries( filteractionaddheadertest
   kdepim
   mailcommon
   pimcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractiondeletetest_SRCS filteractiondeletetest.cpp
                                           ../filteractions/filteractiondelete.cpp
                                           ../filteractions/filteractionwithnone.cpp)
kde4_add_unit_test( filteractiondeletetest ${mailcommon_filteractiondeletetest_SRCS} )
target_link_libraries( filteractiondeletetest
   kdepim
   mailcommon
   pimcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionrewriteheadertest_SRCS filteractionrewriteheadertest.cpp
                                           ../filteractions/filteractionrewriteheader.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionrewriteheadertest ${mailcommon_filteractionrewriteheadertest_SRCS} )
target_link_libraries( filteractionrewriteheadertest
   kdepim
   mailcommon
   pimcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)
