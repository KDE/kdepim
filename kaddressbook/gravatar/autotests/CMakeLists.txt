set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( gravatarcreatejobtest.cpp "../../kaddressbook_debug.cpp;../job/gravatarcreatejob.cpp" "kaddressbookprivate;pimcommon")
add_akonadi_isolated_test_advanced( gravatarupdatejobtest.cpp "../../kaddressbook_debug.cpp;../job/gravatarupdatejob.cpp" "kaddressbookprivate;pimcommon;KF5::Contacts")
add_akonadi_isolated_test_advanced( gravatarcreatewidgettest.cpp "../../kaddressbook_debug.cpp;../widgets/gravatarcreatewidget.cpp" "kaddressbookprivate;pimcommon")
add_akonadi_isolated_test_advanced( gravatarcreatedialogtest.cpp "../../kaddressbook_debug.cpp;../widgets/gravatarcreatedialog.cpp;../widgets/gravatarcreatewidget.cpp" "kaddressbookprivate;pimcommon")

add_akonadi_isolated_test_advanced( gravatarupdatedialogtest.cpp "../widgets/gravatarupdatedialog.cpp;../widgets/gravatarupdatewidget.cpp" "kaddressbookprivate;pimcommon")
add_akonadi_isolated_test_advanced( gravatarupdatewidgettest.cpp "../widgets/gravatarupdatewidget.cpp" "kaddressbookprivate;pimcommon")
