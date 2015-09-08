# convenience macro to add qtest unit tests
macro(add_messageviewer_header_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} 
        TEST_NAME ${_name}
        NAME_PREFIX "messageviewer-header"
        LINK_LIBRARIES messageviewer kleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore )
endmacro ()

#add_messageviewer_header_unittest( htmlquotecolorertest.cpp )

