# convenience macro to add qtest unit tests
macro(messagelist_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} 
        TEST_NAME ${_name}
        NAME_PREFIX "messagelist-"
        LINK_LIBRARIES Qt5::Test KF5::MessageCore KF5::MessageList KF5::PimCommon KF5::Completion
    )
endmacro ()

messagelist_unittest(itemsizetest.cpp)
messagelist_unittest(quicksearchlinetest.cpp)
messagelist_unittest(quicksearchwarningtest.cpp)
