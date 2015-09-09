macro(add_mailimporter_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "mailimporter-"
        LINK_LIBRARIES KF5::MailImporter KF5::AkonadiCore Qt5::Test Qt5::Core Qt5::Gui
    )
endmacro ()

add_mailimporter_unittest(filterthunderbirdtest.cpp)
add_mailimporter_unittest(filtericedovetest.cpp)
add_mailimporter_unittest(filterclawsmailtest.cpp)
add_mailimporter_unittest(filtersylpheedtest.cpp)

