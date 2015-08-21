# convenience macro to add qtest unit tests
macro(messagelist_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ../messagelist_debug.cpp
        TEST_NAME ${_name}
        NAME_PREFIX "messagelist-"
        LINK_LIBRARIES Qt5::Test messagecore messagelist pimcommon
    )
endmacro ()

messagelist_unittest(itemsizetest.cpp)
messagelist_unittest(quicksearchlinetest.cpp)
messagelist_unittest(quicksearchwarningtest.cpp)
