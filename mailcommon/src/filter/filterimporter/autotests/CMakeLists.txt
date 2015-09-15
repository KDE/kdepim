set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced(filterimportclawmailtest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
add_akonadi_isolated_test_advanced(filterimportthunderbirdtest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
add_akonadi_isolated_test_advanced(filterimportbalsatest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
add_akonadi_isolated_test_advanced(filterimportevolutiontest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
add_akonadi_isolated_test_advanced(filterimportprocmailtest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
add_akonadi_isolated_test_advanced(filterimportsylpheedtest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
add_akonadi_isolated_test_advanced(filterimportergmailtest.cpp filtertestkernel.cpp "KF5::MailCommon;KF5::MessageComposer")
