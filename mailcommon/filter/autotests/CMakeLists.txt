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

set(mailcommon_filteractionbeeptest_SRCS filteractionbeeptest.cpp
                                           ../filteractions/filteractionbeep.cpp
                                           ../filteractions/filteractionwithnone.cpp)
kde4_add_unit_test( filteractionbeeptest ${mailcommon_filteractionbeeptest_SRCS} )
target_link_libraries( filteractionbeeptest
   kdepim
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
)

set(mailcommon_filteractionsetstatustest_SRCS filteractionsetstatustest.cpp
                                           ../filteractions/filteractionsetstatus.cpp
                                            ../filteractions/filteractionstatus.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionsetstatustest ${mailcommon_filteractionsetstatustest_SRCS} )
target_link_libraries( filteractionsetstatustest
   kdepim
   pimcommon
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionreplytotest_SRCS filteractionreplytotest.cpp
                                           ../filteractions/filteractionreplyto.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionreplytotest ${mailcommon_filteractionreplytotest_SRCS} )
target_link_libraries( filteractionreplytotest
   kdepim
   pimcommon
   mailcommon
   messagecore
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionwithurltest_SRCS filteractionwithurltest.cpp
                                           ../filteractions/filteractionwithurl.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionwithurltest ${mailcommon_filteractionwithurltest_SRCS} )
target_link_libraries( filteractionwithurltest
   kdepim
   pimcommon
   mailcommon
   messagecore
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionremoveheadertest_SRCS filteractionremoveheadertest.cpp
                                           ../filteractions/filteractionremoveheader.cpp
                                            ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionremoveheadertest ${mailcommon_filteractionremoveheadertest_SRCS} )
target_link_libraries( filteractionremoveheadertest
   kdepim
   pimcommon
   mailcommon
   messagecore
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionwithaddresstest_SRCS filteractionwithaddresstest.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionwithaddresstest ${mailcommon_filteractionwithaddresstest_SRCS} )
target_link_libraries( filteractionwithaddresstest
   kdepim
   pimcommon
   mailcommon
   messagecore
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set(mailcommon_filteractionaddtoaddressbooktest_SRCS filteractionaddtoaddressbooktest.cpp
                                           ../filteractions/filteractionaddtoaddressbook.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionaddtoaddressbooktest ${mailcommon_filteractionaddtoaddressbooktest_SRCS} )
target_link_libraries( filteractionaddtoaddressbooktest
   kdepim
   pimcommon
   mailcommon
   messagecore
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


#set(mailcommon_filteractionplaysoundtest_SRCS filteractionplaysoundtest.cpp
#                                           ../filteractions/filteractionplaysound.cpp
#                                           ../filteractions/filteractionwithtest.cpp
#                                           ../filteractions/filteractionplaysound.h
#                                           ../soundtestwidget.cpp
#                                           ../dialog/filteractionmissingargumentdialog.h)
#kde4_add_unit_test( filteractionplaysoundtest ${mailcommon_filteractionplaysoundtest_SRCS} )
#target_link_libraries( filteractionplaysoundtest
#   kdepim
#   pimcommon
#   mailcommon
#   ${QT_QTTEST_LIBRARY}
#   ${QT_QTCORE_LIBRARY}
#   ${KDEPIMLIBS_AKONADI_LIBS}
#   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
#)

set( mailcommon_invalidfilterdialog_source invalidfilterdialogtest.cpp  )
kde4_add_unit_test( invalidfilterdialogtest ${mailcommon_invalidfilterdialog_source})
target_link_libraries( invalidfilterdialogtest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} mailcommon)

set( mailcommon_invalidfilterwidget_source invalidfilterwidgettest.cpp  )
kde4_add_unit_test( invalidfilterwidgettest ${mailcommon_invalidfilterwidget_source})
target_link_libraries( invalidfilterwidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} mailcommon)

set( mailcommon_invalidfilterlistwidget_source invalidfilterlistwidgettest.cpp  )
kde4_add_unit_test( invalidfilterlistwidgettest ${mailcommon_invalidfilterlistwidget_source})
target_link_libraries( invalidfilterlistwidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} mailcommon)
