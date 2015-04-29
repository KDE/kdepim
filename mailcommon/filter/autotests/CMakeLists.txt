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


set(mailcommon_filteractionplaysoundtest_SRCS filteractionplaysoundtest.cpp
                                           ../filteractions/filteractionplaysound.cpp
                                           ../filteractions/filteractionwithtest.cpp
                                           ../filteractions/filteractionplaysound.cpp
                                           ../soundtestwidget.cpp
                                           ../dialog/filteractionmissingargumentdialog.cpp
                                           ../kmfilteraccountlist.cpp)
kde4_add_unit_test( filteractionplaysoundtest ${mailcommon_filteractionplaysoundtest_SRCS} )
target_link_libraries( filteractionplaysoundtest
   kdepim
   pimcommon
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
   ${KDEPIMLIBS_MAILTRANSPORT_LIBS}

)

set( mailcommon_invalidfilterdialog_source invalidfilterdialogtest.cpp  )
kde4_add_unit_test( invalidfilterdialogtest ${mailcommon_invalidfilterdialog_source})
target_link_libraries( invalidfilterdialogtest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} mailcommon)

set( mailcommon_invalidfilterwidget_source invalidfilterwidgettest.cpp  )
kde4_add_unit_test( invalidfilterwidgettest ${mailcommon_invalidfilterwidget_source})
target_link_libraries( invalidfilterwidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} mailcommon)

set( mailcommon_invalidfilterlistwidget_source invalidfilterlistwidgettest.cpp  )
kde4_add_unit_test( invalidfilterlistwidgettest ${mailcommon_invalidfilterlistwidget_source})
target_link_libraries( invalidfilterlistwidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} mailcommon)


set(mailcommon_filteractionunsetstatustest_SRCS filteractionunsetstatustest.cpp
                                           ../filteractions/filteractionunsetstatus.cpp
                                            ../filteractions/filteractionstatus.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionunsetstatustest ${mailcommon_filteractionunsetstatustest_SRCS} )
target_link_libraries( filteractionunsetstatustest
   kdepim
   pimcommon
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionredirecttest_SRCS filteractionredirecttest.cpp
                                           ../filteractions/filteractionredirect.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionredirecttest ${mailcommon_filteractionredirecttest_SRCS} )
target_link_libraries( filteractionredirecttest
   kdepim
   pimcommon
   mailcommon
   messagecore
   messagecomposer
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set( mailcommon_invalidfilterinfotest_source invalidfilterinfotest.cpp  )
kde4_add_unit_test( invalidfilterinfotest ${mailcommon_invalidfilterinfotest_source})
target_link_libraries( invalidfilterinfotest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} mailcommon)


set(mailcommon_filteractionsendfakedispositiontest_SRCS filteractionsendfakedispositiontest.cpp
                                           ../filteractions/filteractionsendfakedisposition.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
kde4_add_unit_test( filteractionsendfakedispositiontest ${mailcommon_filteractionsendfakedispositiontest_SRCS} )
target_link_libraries( filteractionsendfakedispositiontest
   kdepim
   pimcommon
   mailcommon
   messagecore
   messagecomposer
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionforwardtest_SRCS filteractionforwardtest.cpp
                                           ../filteractions/filteractionforward.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp
                                           ../dialog/filteractionmissingargumentdialog.cpp
                                           ../kmfilteraccountlist.cpp)
kde4_add_unit_test( filteractionforwardtest ${mailcommon_filteractionforwardtest_SRCS} )
target_link_libraries( filteractionforwardtest
   kdepim
   pimcommon
   mailcommon
   messagecore
   messagecomposer
   templateparser
   ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set( mailcommon_invalidfilterinfowidgettest_source invalidfilterinfowidgettest.cpp ../invalidfilters/invalidfilterinfowidget.cpp )
kde4_add_unit_test( invalidfilterinfowidgettest ${mailcommon_invalidfilterinfowidgettest_source})
target_link_libraries( invalidfilterinfowidgettest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})



set(mailcommon_filteractioncopytest_SRCS filteractioncopytest.cpp
                                           ../filteractions/filteractioncopy.cpp
                                           ../filteractions/filteractionwithfolder.cpp)
kde4_add_unit_test( filteractioncopytest ${mailcommon_filteractioncopytest_SRCS} )
target_link_libraries( filteractioncopytest
   kdepim
   pimcommon
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)

set(mailcommon_filteractionmovetest_SRCS filteractionmovetest.cpp
                                           ../filteractions/filteractionmove.cpp
                                           ../filteractions/filteractionwithfolder.cpp)
kde4_add_unit_test( filteractionmovetest ${mailcommon_filteractionmovetest_SRCS} )
target_link_libraries( filteractionmovetest
   kdepim
   pimcommon
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set(mailcommon_filteractionexectest_SRCS filteractionexectest.cpp
                                         ../filteractions/filteractionexec.cpp
                                         ../filteractions/filteractionwithcommand.cpp
                                         ../filteractions/filteractionwithurl.cpp)

kde4_add_unit_test( filteractionexectest ${mailcommon_filteractionexectest_SRCS} )
target_link_libraries( filteractionexectest
   kdepim
   pimcommon
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set(mailcommon_filteractionsendreceipttest_SRCS filteractionsendreceipttest.cpp
                                           ../filteractions/filteractionsendreceipt.cpp
                                           ../filteractions/filteractionwithnone.cpp)

kde4_add_unit_test( filteractionsendreceipttest ${mailcommon_filteractionsendreceipttest_SRCS} )
target_link_libraries( filteractionsendreceipttest
   kdepim
   pimcommon
   mailcommon
   messagecomposer
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set(mailcommon_filteractionsettransporttest_SRCS filteractionsettransporttest.cpp ../filteractions/filteractionsettransport.cpp
                                           ../dialog/filteractionmissingargumentdialog.cpp
                                           ../kmfilteraccountlist.cpp)


kde4_add_unit_test( filteractionsettransporttest ${mailcommon_filteractionsettransporttest_SRCS} )
target_link_libraries( filteractionsettransporttest
   kdepim
   pimcommon
   mailcommon
   ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDEPIMLIBS_AKONADI_LIBS}
   ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
)


set(mailcommon_mailfiltertest_SRCS mailfiltertest.cpp )

kde4_add_unit_test( mailfiltertest ${mailcommon_mailfiltertest_SRCS} )
target_link_libraries( mailfiltertest
   mailcommon
   ${QT_QTTEST_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${KDE4_KDEUI_LIBRARY}   
)

