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
