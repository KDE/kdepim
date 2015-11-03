set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

include( ${CMAKE_SOURCE_DIR}/cmake/modules/kdepim_add_gpg_crypto_test.cmake )

# Convenience macro to add messagecomposer unit tests.
macro( add_messagecomposer_test _source )
    ecm_add_test(${_source}
        NAME_PREFIX "messagecomposer-"
        LINK_LIBRARIES KF5::Mime KF5::MessageComposer KF5::MessageViewer KF5::MessageCore KF5::TemplateParser
                       KF5::PimTextEdit Qt5::Test KF5::IdentityManagement KF5::Contacts KF5::AkonadiCore KF5::XmlGui KF5::Libkdepim KF5::IconThemes KF5::Completion
    )
endmacro()

macro( add_messagecomposer_cryptotest _source )
    set( _test cryptofunctions.cpp setupenv.cpp ${_source} )
    get_filename_component( _name ${_source} NAME_WE )
    add_executable( ${_name} ${_test} )
    ecm_mark_as_test(messagecomposer-${_name})
    target_link_libraries(
        ${_name}
        KF5::Mime
        KF5::MessageComposer
        KF5::MessageViewer
        KF5::MessageCore
        KF5::Libkleo
        KF5::Libkdepim
        KF5::PimTextEdit
        KF5::AkonadiCore
        Qt5::Test
        KF5::QGpgme
        KF5::IconThemes
        KF5::Completion
        KF5::MailTransport
        KF5::Ldap
    )
    add_gpg_crypto_test(${_name} ${_name})
endmacro()


# Utility stuff.
add_messagecomposer_test( utiltest.cpp )
add_messagecomposer_test( messagefactorytest.cpp )

# Non-content jobs.
add_messagecomposer_test( skeletonmessagejobtest.cpp )

# Basic content jobs.
add_messagecomposer_test( singlepartjobtest.cpp )
add_messagecomposer_test( multipartjobtest.cpp )

# More complex content jobs.
add_messagecomposer_test( attachmentjobtest.cpp )
add_messagecomposer_test( maintextjobtest.cpp )

add_messagecomposer_test( attachmentvcardfromaddressbookjobtest.cpp )

# Composer.
add_messagecomposer_test( composertest.cpp )
add_messagecomposer_cryptotest( cryptocomposertest.cpp )
add_messagecomposer_test( infoparttest.cpp )
add_messagecomposer_test( textparttest.cpp )
add_messagecomposer_test( globalparttest.cpp )
add_messagecomposer_test( composerviewbasetest.cpp )

# Crypto
add_messagecomposer_cryptotest( signjobtest.cpp )
add_messagecomposer_cryptotest( encryptjobtest.cpp )
add_messagecomposer_cryptotest( signencrypttest.cpp )

set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( followupreminderselectdatedialogtest.cpp  "../src/followupreminder/followupreminderselectdatedialog.cpp" "KF5::CalendarCore;KF5::AkonadiCore;KF5::AkonadiWidgets;KF5::MessageComposer")

