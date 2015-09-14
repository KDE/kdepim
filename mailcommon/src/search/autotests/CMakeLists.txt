macro(add_search_autotest _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "mailcommon-search-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::Mime KF5::AkonadiCore KF5::AkonadiMime KF5::MailCommon
    )
endmacro()

add_search_autotest(searchpatterntest
    searchpatterntest.cpp
)
add_search_autotest(searchruletest
    searchruletest.cpp
)
add_search_autotest(searchrulestatustest
    searchrulestatustest.cpp
    ../searchrule/searchrulestatus.cpp
)
add_search_autotest(searchrulenumericaltest
    searchrulenumericaltest.cpp
    ../searchrule/searchrulenumerical.cpp
)
add_search_autotest(searchruledatetest
    searchruledatetest.cpp
    ../searchrule/searchruledate.cpp
)
add_search_autotest(searchrulestringtest
    searchrulestringtest.cpp
    ../searchrule/searchrulestring.cpp
)
