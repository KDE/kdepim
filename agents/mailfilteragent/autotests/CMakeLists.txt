# Convenience macro to add unit tests.
macro(add_mailfilter_agent_test _source )
    set(_test ${_source})
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_test}
        TEST_NAME ${_name}
        NAME_PREFIX "mailfilteragent-"
        LINK_LIBRARIES Qt5::Test Qt5::Widgets
    )
endmacro()

add_mailfilter_agent_test(configuredialogtest.cpp)
