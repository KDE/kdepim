macro(add_libkdepim_addressline_unittest _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "libkdepim-addressline-"
        LINK_LIBRARIES Qt5::Test KF5::Contacts Qt5::Widgets KF5::Libkdepim Qt5::DBus KF5::Codecs KF5::ConfigCore KF5::Completion KF5::I18n KF5::WidgetsAddons
    )
endmacro()

add_libkdepim_addressline_unittest(baloocompletionmailtest
    baloocompletionemailtest.cpp
    ../addresslineedit/baloocompletionemail.cpp
)
add_libkdepim_addressline_unittest(recentaddresswidgettest
    recentaddresswidgettest.cpp
    ../recentaddress/recentaddresswidget.cpp
    ../recentaddress/recentaddresses.cpp
)
add_libkdepim_addressline_unittest(recentaddressdialogtest
    recentaddressdialogtest.cpp
    ../recentaddress/recentaddressdialog.cpp
    ../recentaddress/recentaddresswidget.cpp
    ../recentaddress/recentaddresses.cpp
)
add_libkdepim_addressline_unittest(completionconfiguredialogtest
    completionconfiguredialogtest.cpp
)
add_libkdepim_addressline_unittest(completionorderwidgettest
    completionorderwidgettest.cpp
)
