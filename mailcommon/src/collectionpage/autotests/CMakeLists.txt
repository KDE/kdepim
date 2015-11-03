macro(add_mailcommon_collectionpage_test _name)
    ecm_add_test(${ARGN}
        TEST_NAME ${_name}
        NAME_PREFIX "mailcommon-collectionpage-"
        LINK_LIBRARIES Qt5::Test Qt5::Gui KF5::AkonadiCore KF5::MailCommon KF5::Completion
    )
endmacro()

add_mailcommon_collectionpage_test(collectionannotationsattributetest
    collectionannotationsattributetest.cpp
    ../attributes/collectionannotationsattribute.cpp
)
add_mailcommon_collectionpage_test(expirecollectionattributetest
    expirecollectionattributetest.cpp
    ../attributes/expirecollectionattribute.cpp
)
add_mailcommon_collectionpage_test(incidencesforwidgettest
    incidencesforwidgettest.cpp
)
add_mailcommon_collectionpage_test(contenttypewidgettest
    contenttypewidgettest.cpp
)
