add_subdirectory(gnupg_home)

# Convenience macro to add unit tests.
macro(add_messagecore_test _source )
    add_definitions(-DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
    ecm_add_test(${_source}
        NAME_PREFIX "messagecore-"
        LINK_LIBRARIES Qt5::Test kleo messagecore
    )
endmacro()

# Attachment tests.
add_messagecore_test( attachmentcompressjobtest.cpp )
add_messagecore_test( attachmentfrommimecontentjobtest.cpp )
add_messagecore_test( attachmentfromurljobtest.cpp )
add_messagecore_test( attachmentparttest.cpp )
add_messagecore_test( attachmentpropertiesdialogtest.cpp )
add_messagecore_test( attachmentupdatejobtest.cpp )

# Other tests
add_messagecore_test( stringutiltest.cpp )
add_messagecore_test( mailinglisttest.cpp )

ecm_add_test(mdnstateattributetest.cpp ../misc/mdnstateattribute.cpp
    TEST_NAME msdnattributetest
    NAME_PREFIX "messagecore-"
    LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::AkonadiCore messagecore
)
