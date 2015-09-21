add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

add_definitions(-DKMAIL_UNITTESTS=YES)

# convenience macro to add qtest unit tests
macro(add_messageviewer_gravatar_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "messageviewer-gravatar-"
        LINK_LIBRARIES KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore
    )
endmacro ()

add_messageviewer_gravatar_unittest( gravatarconfigwidgettest.cpp )
add_messageviewer_gravatar_unittest( gravatarconfiguresettingsdialogtest.cpp )
