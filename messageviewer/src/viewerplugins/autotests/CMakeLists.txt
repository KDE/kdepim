macro(add_messageviewer_viewerplugin_class_unittest _source )
    set( _test ${_source})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    add_test( ${_name} ${_name} )

    ecm_mark_as_test(messageviewer-viewerplugin-${_name})
    target_link_libraries( ${_name} KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore KF5::GrantleeTheme)
endmacro ()

add_messageviewer_viewerplugin_class_unittest(viewerpluginmanagertest.cpp)
