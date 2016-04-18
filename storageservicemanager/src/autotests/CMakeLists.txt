include_directories( 
    ${storageservicemanager_BINARY_DIR}/src
    )


# convenience macro to add qtest unit tests
macro(add_storageservice_unittest _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "storageservicemanager-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui Qt5::Widgets KF5::I18n KF5::ConfigGui
        )
endmacro()

add_storageservice_unittest(navigationbuttontest
    storageservicenavigationbuttontest.cpp
    ../storageservicenavigationbuttons.cpp
    ../storageservicemanagerutil.cpp
    ${storageservicemanager_BINARY_DIR}/src/storageservicemanager_debug.cpp
    )
add_storageservice_unittest(configurestoragewidgettest
    storageserviceconfigurestoragewidgettest.cpp
    ../storageserviceconfigurestoragewidget.cpp
    ${storageservicemanager_BINARY_DIR}/src/storageservicemanager_debug.cpp
    )
