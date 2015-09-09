# convenience macro to add qtest unit tests

macro(add_messageviewer_header_unittest _source _additional _jsonfile)
    set( _test ${_source}  ${_additional})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    kcoreaddons_desktop_to_json(${_name} ${_jsonfile})
    add_test( ${_name} ${_name} )
    
    ecm_mark_as_test(messageviewer-header-${_name})
    target_link_libraries( ${_name} messageviewer kleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore grantleetheme)

endmacro ()

add_messageviewer_header_unittest( briefheaderstyleplugintest.cpp "../briefheaderstyleplugin/briefheaderstyleplugin.cpp;../briefheaderstyleplugin/briefheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../briefheaderstyleplugin/messageviewer_briefheaderstyleplugin.desktop")

