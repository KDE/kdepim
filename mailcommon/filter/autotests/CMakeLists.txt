set( mailcommon_itemcontexttest_source itemcontexttest.cpp ../itemcontext.cpp )
add_executable( itemcontexttest ${mailcommon_itemcontexttest_source})
add_test(itemcontexttest itemcontexttest)
ecm_mark_as_test(itemcontexttest)
target_link_libraries( itemcontexttest Qt5::Test Qt5::Gui KF5::AkonadiCore mailcommon)

set( mailcommon_filteractionpipethroughtest_source
   ../../mailcommon_debug.cpp
   filteractionpipethroughtest.cpp
   ../filteractions/filteractionpipethrough.cpp
   ../filteractions/filteractionwithcommand.cpp
   ../filteractions/filteractionwithurl.cpp
)
add_executable( filteractionpipethroughtest ${mailcommon_filteractionpipethroughtest_source} )
add_test(filteractionpipethroughtest filteractionpipethroughtest)
ecm_mark_as_test(filteractionpipethroughtest)
target_link_libraries( filteractionpipethroughtest
   kdepim
   mailcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionheadertest_SRCS filteractionaddheadertest.cpp ../../mailcommon_debug.cpp
                                           ../filteractions/filteractionaddheader.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionaddheadertest ${mailcommon_filteractionheadertest_SRCS} )
add_test(filteractionaddheadertest filteractionaddheadertest)
ecm_mark_as_test(filteractionaddheadertest)
target_link_libraries( filteractionaddheadertest
   kdepim
   mailcommon
   pimcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractiondeletetest_SRCS filteractiondeletetest.cpp
                                           ../filteractions/filteractiondelete.cpp
                                           ../filteractions/filteractionwithnone.cpp)
add_executable( filteractiondeletetest ${mailcommon_filteractiondeletetest_SRCS} )
add_test(filteractiondeletetest filteractiondeletetest)
ecm_mark_as_test(filteractiondeletetest)
target_link_libraries( filteractiondeletetest
   kdepim
   mailcommon
   pimcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionrewriteheadertest_SRCS filteractionrewriteheadertest.cpp
                                           ../filteractions/filteractionrewriteheader.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionrewriteheadertest ${mailcommon_filteractionrewriteheadertest_SRCS} )
add_test(filteractionrewriteheadertest filteractionrewriteheadertest)
ecm_mark_as_test(filteractionrewriteheadertest)
target_link_libraries( filteractionrewriteheadertest
   kdepim
   mailcommon
   pimcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionbeeptest_SRCS filteractionbeeptest.cpp
                                           ../filteractions/filteractionbeep.cpp
                                           ../filteractions/filteractionwithnone.cpp)
add_executable( filteractionbeeptest ${mailcommon_filteractionbeeptest_SRCS} )
add_test(filteractionbeeptest filteractionbeeptest)
ecm_mark_as_test(filteractionbeeptest)
target_link_libraries( filteractionbeeptest
   kdepim
   mailcommon
   Qt5::Test
   Qt5::Core
)

set(mailcommon_filteractionsetstatustest_SRCS filteractionsetstatustest.cpp ../../mailcommon_debug.cpp
                                           ../filteractions/filteractionsetstatus.cpp
                                            ../filteractions/filteractionstatus.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionsetstatustest ${mailcommon_filteractionsetstatustest_SRCS} )
add_test(filteractionsetstatustest filteractionsetstatustest)
ecm_mark_as_test(filteractionsetstatustest)
target_link_libraries( filteractionsetstatustest
   kdepim
   pimcommon
   mailcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionreplytotest_SRCS filteractionreplytotest.cpp
                                           ../filteractions/filteractionreplyto.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionreplytotest ${mailcommon_filteractionreplytotest_SRCS} )
add_test(filteractionreplytotest filteractionreplytotest)
ecm_mark_as_test(filteractionreplytotest)
target_link_libraries( filteractionreplytotest
   kdepim
   pimcommon
   mailcommon
   messagecore
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionwithurltest_SRCS filteractionwithurltest.cpp
                                           ../filteractions/filteractionwithurl.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionwithurltest ${mailcommon_filteractionwithurltest_SRCS} )
add_test(filteractionwithurltest filteractionwithurltest)
ecm_mark_as_test(filteractionwithurltest)
target_link_libraries( filteractionwithurltest
   kdepim
   pimcommon
   mailcommon
   messagecore
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionremoveheadertest_SRCS filteractionremoveheadertest.cpp
                                           ../filteractions/filteractionremoveheader.cpp
                                            ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionremoveheadertest ${mailcommon_filteractionremoveheadertest_SRCS} )
add_test(filteractionremoveheadertest filteractionremoveheadertest)
ecm_mark_as_test(filteractionremoveheadertest)
target_link_libraries( filteractionremoveheadertest
   kdepim
   pimcommon
   mailcommon
   messagecore
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionwithaddresstest_SRCS filteractionwithaddresstest.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionwithaddresstest ${mailcommon_filteractionwithaddresstest_SRCS} )
add_test(filteractionwithaddresstest filteractionwithaddresstest)
ecm_mark_as_test(filteractionwithaddresstest)
target_link_libraries( filteractionwithaddresstest
   kdepim
   pimcommon
   mailcommon
   messagecore
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)


set(mailcommon_filteractionaddtoaddressbooktest_SRCS filteractionaddtoaddressbooktest.cpp
                                           ../filteractions/filteractionaddtoaddressbook.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionaddtoaddressbooktest ${mailcommon_filteractionaddtoaddressbooktest_SRCS} )
add_test(filteractionaddtoaddressbooktest filteractionaddtoaddressbooktest)
ecm_mark_as_test(filteractionaddtoaddressbooktest)
target_link_libraries( filteractionaddtoaddressbooktest
   kdepim
   pimcommon
   mailcommon
   messagecore
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)


set(mailcommon_filteractionplaysoundtest_SRCS filteractionplaysoundtest.cpp
                                           ../filteractions/filteractionplaysound.cpp
                                           ../filteractions/filteractionwithtest.cpp
                                           ../filteractions/filteractionplaysound.cpp
                                           ../soundtestwidget.cpp
                                           ../dialog/filteractionmissingargumentdialog.cpp
                                           ../kmfilteraccountlist.cpp)
add_executable( filteractionplaysoundtest ${mailcommon_filteractionplaysoundtest_SRCS} )
add_test(filteractionplaysoundtest filteractionplaysoundtest)
ecm_mark_as_test(filteractionplaysoundtest)
target_link_libraries( filteractionplaysoundtest
   kdepim
   pimcommon
   mailcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
   KF5::MailTransport
)

set( mailcommon_invalidfilterdialog_source invalidfilterdialogtest.cpp  )
add_executable( invalidfilterdialogtest ${mailcommon_invalidfilterdialog_source})
add_test(invalidfilterdialogtest invalidfilterdialogtest)
ecm_mark_as_test(invalidfilterdialogtest)
target_link_libraries( invalidfilterdialogtest Qt5::Test KF5::KDELibs4Support Qt5::Gui KF5::AkonadiCore mailcommon)

set( mailcommon_invalidfilterwidget_source invalidfilterwidgettest.cpp  )
add_executable( invalidfilterwidgettest ${mailcommon_invalidfilterwidget_source})
add_test(invalidfilterwidgettest invalidfilterwidgettest)
ecm_mark_as_test(invalidfilterwidgettest)
target_link_libraries( invalidfilterwidgettest Qt5::Test KF5::KDELibs4Support Qt5::Gui KF5::AkonadiCore mailcommon)

set( mailcommon_invalidfilterlistwidget_source invalidfilterlistwidgettest.cpp  )
add_executable( invalidfilterlistwidgettest ${mailcommon_invalidfilterlistwidget_source})
add_test(invalidfilterlistwidgettest invalidfilterlistwidgettest)
ecm_mark_as_test(invalidfilterlistwidgettest)
target_link_libraries( invalidfilterlistwidgettest Qt5::Test KF5::KDELibs4Support Qt5::Gui KF5::AkonadiCore mailcommon)


set(mailcommon_filteractionunsetstatustest_SRCS filteractionunsetstatustest.cpp ../../mailcommon_debug.cpp
                                           ../filteractions/filteractionunsetstatus.cpp
                                            ../filteractions/filteractionstatus.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionunsetstatustest ${mailcommon_filteractionunsetstatustest_SRCS} )
add_test(filteractionunsetstatustest filteractionunsetstatustest)
ecm_mark_as_test(filteractionunsetstatustest)
target_link_libraries( filteractionunsetstatustest
   kdepim
   pimcommon
   mailcommon
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionredirecttest_SRCS filteractionredirecttest.cpp ../../mailcommon_debug.cpp
                                           ../filteractions/filteractionredirect.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionredirecttest ${mailcommon_filteractionredirecttest_SRCS} )
add_test(filteractionredirecttest filteractionredirecttest)
ecm_mark_as_test(filteractionredirecttest)
target_link_libraries( filteractionredirecttest
   kdepim
   pimcommon
   mailcommon
   messagecore
   messagecomposer
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)


set( mailcommon_invalidfilterinfotest_source invalidfilterinfotest.cpp  )
add_executable( invalidfilterinfotest ${mailcommon_invalidfilterinfotest_source})
add_test(invalidfilterinfotest invalidfilterinfotest)
ecm_mark_as_test(invalidfilterinfotest)
target_link_libraries( invalidfilterinfotest Qt5::Test  mailcommon)


set(mailcommon_filteractionsendfakedispositiontest_SRCS filteractionsendfakedispositiontest.cpp
                                           ../filteractions/filteractionsendfakedisposition.cpp
                                           ../filteractions/filteractionwithstringlist.cpp
                                           ../filteractions/filteractionwithstring.cpp)
add_executable( filteractionsendfakedispositiontest ${mailcommon_filteractionsendfakedispositiontest_SRCS} )
add_test(filteractionsendfakedispositiontest filteractionsendfakedispositiontest)
ecm_mark_as_test(filteractionsendfakedispositiontest)
target_link_libraries( filteractionsendfakedispositiontest
   kdepim
   pimcommon
   mailcommon
   messagecore
   messagecomposer
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)

set(mailcommon_filteractionforwardtest_SRCS filteractionforwardtest.cpp ../../mailcommon_debug.cpp
                                           ../filteractions/filteractionforward.cpp
                                           ../filteractions/filteractionwithaddress.cpp
                                           ../filteractions/filteractionwithstring.cpp
                                           ../dialog/filteractionmissingargumentdialog.cpp
                                           ../kmfilteraccountlist.cpp)
add_executable( filteractionforwardtest ${mailcommon_filteractionforwardtest_SRCS} )
add_test(filteractionforwardtest filteractionforwardtest)
ecm_mark_as_test(filteractionforwardtest)
target_link_libraries( filteractionforwardtest
   kdepim
   pimcommon
   mailcommon
   messagecore
   messagecomposer
   templateparser
   KF5::MailTransport
   Qt5::Test
   Qt5::Core
   KF5::AkonadiCore
   KF5::AkonadiMime
)


set( mailcommon_invalidfilterinfowidgettest_source invalidfilterinfowidgettest.cpp ../invalidfilters/invalidfilterinfowidget.cpp )
add_executable( invalidfilterinfowidgettest ${mailcommon_invalidfilterinfowidgettest_source})
add_test(invalidfilterinfowidgettest invalidfilterinfowidgettest)
ecm_mark_as_test(invalidfilterinfowidgettest)
target_link_libraries( invalidfilterinfowidgettest Qt5::Test  Qt5::Gui KF5::WidgetsAddons mailcommon)
