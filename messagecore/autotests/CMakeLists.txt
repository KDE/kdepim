add_subdirectory(gnupg_home)

# Convenience macro to add unit tests.
macro(add_messagecore_test _source )
    add_definitions(-DKDESRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
    ecm_add_test(${_source}
        NAME_PREFIX "messagecore-"
        LINK_LIBRARIES Qt5::Test KF5::Libkleo KF5::MessageCore Qt5::Widgets KF5::Archive KF5::Completion KF5::ConfigCore KF5::CoreAddons
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
