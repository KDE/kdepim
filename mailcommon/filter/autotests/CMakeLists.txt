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
