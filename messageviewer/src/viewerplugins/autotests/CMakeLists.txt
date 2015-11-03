
set(common_SRCS)
ecm_qt_declare_logging_category(common_SRCS HEADER createeventplugin_debug.h IDENTIFIER CREATEEVENTPLUGIN_LOG CATEGORY_NAME log_createeventplugin)
ecm_qt_declare_logging_category(common_SRCS HEADER createnoteplugin_debug.h IDENTIFIER CREATENOTEPLUGIN_LOG CATEGORY_NAME log_createnoteplugin)
ecm_qt_declare_logging_category(common_SRCS HEADER createtodoplugin_debug.h IDENTIFIER CREATETODOPLUGIN_LOG CATEGORY_NAME log_createtodoplugin)
macro(add_messageviewer_viewerplugin_unittest _source _additional _jsonfile)
    set( _test ${_source}  ${_additional} ${common_SRCS})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    kcoreaddons_desktop_to_json(${_name} ${_jsonfile})
    add_test( ${_name} ${_name} )

    ecm_mark_as_test(messageviewer-viewerplugin-unittest-${_name})
    target_link_libraries( ${_name} KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore KF5::GrantleeTheme KF5::IncidenceEditorsng)
endmacro ()



macro(add_messageviewer_viewerplugin_class_unittest _source )
    set( _test ${_source} ${common_SRCS})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    add_test( ${_name} ${_name} )

    ecm_mark_as_test(messageviewer-viewerplugin-${_name})
    target_link_libraries( ${_name} KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore KF5::GrantleeTheme)
endmacro ()


macro(add_messageviewer_viewerplugin_widget_unittest _source _additional )
    set( _test ${_source} ${_additional} ${common_SRCS})
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} )
    add_test( ${_name} ${_name} )

    ecm_mark_as_test(messageviewer-viewerplugin-${_name})
    target_link_libraries( ${_name} KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore KF5::GrantleeTheme KF5::IncidenceEditorsng)
endmacro ()


add_messageviewer_viewerplugin_class_unittest(viewerpluginmanagertest.cpp)
add_messageviewer_viewerplugin_class_unittest(viewerplugintoolmanagertest.cpp)
add_messageviewer_viewerplugin_widget_unittest(todoedittest.cpp "../createtodoplugin/todoedit.cpp")
add_messageviewer_viewerplugin_widget_unittest(noteedittest.cpp "../createnoteplugin/noteedit.cpp")
set_tests_properties(todoedittest PROPERTIES ENVIRONMENT "LC_ALL=C")

add_messageviewer_viewerplugin_widget_unittest(eventedittest.cpp "../createeventplugin/eventedit.cpp;../createeventplugin/eventdatetimewidget.cpp")
add_messageviewer_viewerplugin_widget_unittest(eventdatetimewidgettest.cpp "../createeventplugin/eventdatetimewidget.cpp")
add_messageviewer_viewerplugin_unittest(viewerplugincreateeventtest.cpp "../createeventplugin/eventdatetimewidget.cpp;../createeventplugin/eventedit.cpp;../createeventplugin/viewerplugincreateevent.cpp;../createeventplugin/viewerplugincreateeventinterface.cpp" "../createeventplugin/messageviewer_createeventplugin.desktop")
add_messageviewer_viewerplugin_unittest(viewerplugincreatetodotest.cpp "../createtodoplugin/viewerplugincreatetodo.cpp;../createtodoplugin/viewerplugincreatetodointerface.cpp;../createtodoplugin/todoedit.cpp" "../createtodoplugin/messageviewer_createtodoplugin.desktop")
add_messageviewer_viewerplugin_unittest(viewerplugincreatenotetest.cpp "../createnoteplugin/noteedit.cpp;../createnoteplugin/viewerplugincreatenote.cpp;../createnoteplugin/viewerplugincreatenoteinterface.cpp" "../createnoteplugin/messageviewer_createnoteplugin.desktop")
add_messageviewer_viewerplugin_unittest(viewerplugintranslatortest.cpp "../translatorplugin/viewerplugintranslator.cpp;../translatorplugin/viewerplugintranslatorinterface.cpp" "../translatorplugin/messageviewer_translatorplugin.desktop")
