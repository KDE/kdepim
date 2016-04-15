include_directories( 
    ${CMAKE_BINARY_DIR}/storageservicemanager/
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
    ${CMAKE_BINARY_DIR}/storageservicemanager/storageservicemanager_debug.cpp
    )
add_storageservice_unittest(configurestoragewidgettest
    storageserviceconfigurestoragewidgettest.cpp
    ../storageserviceconfigurestoragewidget.cpp
    ${CMAKE_BINARY_DIR}/storageservicemanager/storageservicemanager_debug.cpp
    )
