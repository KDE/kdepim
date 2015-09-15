########### recipientseditor test ###############

set(recipienteditortest_SRCS recipientseditortest.cpp)
add_executable(recipienteditortest ${recipienteditortest_SRCS})
target_link_libraries(recipienteditortest KF5::KIOCore KF5::AkonadiContact KF5::MessageComposer KF5::Libkdepim KF5::Completion KF5::Ldap)


########### recipientseditor test ###############

set(composerlineedittest_SRCS testcomposerlineedit.cpp)
add_executable(composerlineedittest ${composerlineedittest_SRCS})
ecm_mark_as_test(composerlineedittest)
target_link_libraries(composerlineedittest KF5::MessageComposer  KF5::Libkdepim KF5::Completion KF5::Ldap)

