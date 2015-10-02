set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

include(${CMAKE_SOURCE_DIR}/cmake/modules/kdepim_add_gpg_crypto_test.cmake)

add_definitions(-DKMAIL_UNITTESTS=YES)

# convenience macro to add qtest unit tests
macro(add_messageviewer_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} util.cpp setupenv.cpp testcsshelper.cpp ../src/viewer/cryptohelper.cpp
        TEST_NAME ${_name}
        NAME_PREFIX "messageviewer-"
        LINK_LIBRARIES KF5::MessageViewer KF5::Libkleo KF5::QGpgme Qt5::Test Qt5::WebKitWidgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::AkonadiNotes KF5::CalendarCore
    )
endmacro ()

macro(add_messageviewer_class_unittest _source _additionalSource)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_additionalSource}
        TEST_NAME ${_name}
        NAME_PREFIX "messageviewer-"
        LINK_LIBRARIES KF5::MessageViewer Qt5::Test KF5::IconThemes KF5::XmlGui Qt5::WebKitWidgets
    )
endmacro ()

macro(add_messageviewer_crypto_unittest _source)
    set(_test ${_source} util.cpp)
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} setupenv.cpp  testcsshelper.cpp )
    ecm_mark_as_test(messageviewer-${_name})
    target_link_libraries( ${_name}
        KF5::MessageViewer
        KF5::Libkleo
        KF5::QGpgme
        Qt5::Test
        Qt5::WebKitWidgets
        KF5::KIOCore
        KF5::Mime
        KF5::AkonadiCore
        KF5::AkonadiNotes
        KF5::CalendarCore
    )
    add_gpg_crypto_test(${_name} messageviewer-${_name})
endmacro ()

add_messageviewer_unittest( htmlquotecolorertest.cpp )
add_messageviewer_crypto_unittest( objecttreeparsertest.cpp )
add_messageviewer_crypto_unittest( rendertest.cpp )
add_messageviewer_crypto_unittest( unencryptedmessagetest.cpp )
add_messageviewer_crypto_unittest( attachmenttest.cpp )
add_messageviewer_unittest( cryptohelpertest.cpp )
add_messageviewer_unittest( messagedisplayformatattributetest.cpp )
add_messageviewer_unittest( nodehelpertest.cpp )
add_messageviewer_class_unittest(zoomactionmenutest.cpp "../src/widgets/zoomactionmenu.cpp")

# convenience macro to add qtest unit tests
macro(add_messageviewer_mailsourceviewbrowserwidget_unittest _source)
    set(_test ${_source} util.cpp)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} util.cpp ../src/widgets/mailsourceviewtextbrowserwidget.cpp ../src/findbar/findbarbase.cpp ../src/findbar/findbarsourceview.cpp
        TEST_NAME ${_name}
        NAME_PREFIX "messageviewer-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui Qt5::Widgets KF5::KIOCore KF5::Mime KF5::AkonadiCore KF5::CalendarCore KF5::PimTextEdit KF5::PimCommon KF5::MessageViewer
    )
endmacro ()


add_messageviewer_mailsourceviewbrowserwidget_unittest( mailsourceviewtextbrowserwidgettest.cpp )
########### viewertest_gui ###############

ecm_add_test(converthtmltoplaintexttest.cpp ../src/viewer/converthtmltoplaintext.cpp
    TEST_NAME converthtmltoplaintexttest
    NAME_PREFIX "messageviewer-"
    LINK_LIBRARIES Qt5::Test Grantlee5::TextDocument
)

set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( viewertest.cpp "" "KF5::MessageViewer;KF5::XmlGui")

