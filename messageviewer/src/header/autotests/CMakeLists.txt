# convenience macro to add qtest unit tests

macro(add_messageviewer_header_unittest _source _additional _jsonfile)
    set( _test ${_source}  ${_additional})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    kcoreaddons_desktop_to_json(${_name} ${_jsonfile})
    add_test( ${_name} ${_name} )
    
    ecm_mark_as_test(messageviewer-header-${_name})
    target_link_libraries( ${_name} KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore KF5::GrantleeTheme)
endmacro ()


macro(add_messageviewer_header_class_unittest _source )
    set( _test ${_source})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    add_test( ${_name} ${_name} )

    ecm_mark_as_test(messageviewer-header-${_name})
    target_link_libraries( ${_name} KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore KF5::GrantleeTheme)
endmacro ()


add_messageviewer_header_unittest( briefheaderstyleplugintest.cpp "../briefheaderstyleplugin/briefheaderstrategy.cpp;../briefheaderstyleplugin/briefheaderstyle.cpp;../briefheaderstyleplugin/briefheaderstyleplugin.cpp;../briefheaderstyleplugin/briefheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../briefheaderstyleplugin/messageviewer_briefheaderstyleplugin.desktop")

add_messageviewer_header_unittest( allheaderstyleplugintest.cpp "../allheaderstyleplugin/allheaderstrategy.cpp;../allheaderstyleplugin/allheaderstyleplugin.cpp;../allheaderstyleplugin/allheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../allheaderstyleplugin/messageviewer_allheaderstyleplugin.desktop")
add_messageviewer_header_unittest( customheaderstyleplugintest.cpp "../customheaderstyleplugin/customheaderstrategy.cpp;../customheaderstyleplugin/customheaderstyle.cpp;../customheaderstyleplugin/customheaderstyleplugin.cpp;../customheaderstyleplugin/customheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../customheaderstyleplugin/messageviewer_customheaderstyleplugin.desktop")

add_messageviewer_header_unittest( enterpriseheaderstyleplugintest.cpp "../enterpriseheaderstyleplugin/enterpriseheaderstyle.cpp;../enterpriseheaderstyleplugin/enterpriseheaderstyleplugin.cpp;../enterpriseheaderstyleplugin/enterpriseheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../enterpriseheaderstyleplugin/messageviewer_enterpriseheaderstyleplugin.desktop")

add_messageviewer_header_unittest( fancyheaderstyleplugintest.cpp "../fancyheaderstyleplugin/fancyheaderstyle.cpp;../fancyheaderstyleplugin/fancyheaderstyleplugin.cpp;../fancyheaderstyleplugin/fancyheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../fancyheaderstyleplugin/messageviewer_fancyheaderstyleplugin.desktop")

add_messageviewer_header_unittest( grantleeheaderstyleplugintest.cpp "../grantleeheaderstyleplugin/grantleeheaderstrategy.cpp;../grantleeheaderstyleplugin/grantleeheaderstyleplugin.cpp;../grantleeheaderstyleplugin/grantleeheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../grantleeheaderstyleplugin/messageviewer_grantleeheaderstyleplugin.desktop")

add_messageviewer_header_unittest( longheaderstyleplugintest.cpp "../longheaderstyleplugin/longheaderstyleplugin.cpp;../longheaderstyleplugin/longheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../longheaderstyleplugin/messageviewer_longheaderstyleplugin.desktop")

add_messageviewer_header_unittest( standardsheaderstyleplugintest.cpp "../standardsheaderstyleplugin/standardheaderstrategy.cpp;../standardsheaderstyleplugin/standardsheaderstyleplugin.cpp;../standardsheaderstyleplugin/standardsheaderstyleinterface.cpp;../headerstyleinterface.cpp" "../standardsheaderstyleplugin/messageviewer_standardsheaderstyleplugin.desktop")

add_messageviewer_header_class_unittest( headerstylepluginmanagertest.cpp)
