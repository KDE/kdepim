set( mailcommon_itemcontexttest_source itemcontexttest.cpp ../itemcontext.cpp )
add_executable( itemcontexttest ${mailcommon_itemcontexttest_source})
add_test(itemcontexttest itemcontexttest)
ecm_mark_as_test(itemcontexttest)
target_link_libraries( itemcontexttest Qt5::Test Qt5::Gui KF5::AkonadiCore mailcommon)

set( mailcommon_filteractionpipethroughtest_source
   actionpipethrough.cpp
   ../../mailcommon_debug.cpp
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
