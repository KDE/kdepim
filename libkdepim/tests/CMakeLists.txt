########### next target ###############
set(testldapclient_SRCS testldapclient.cpp)

add_executable(testldapclient ${testldapclient_SRCS})

target_link_libraries(testldapclient KF5::Libkdepim KF5::I18n KF5::Completion KF5::Ldap)


########### next target ###############
set(testaddresseelineedit_SRCS testaddresseelineedit.cpp)

add_executable(testaddresseelineedit ${testaddresseelineedit_SRCS})

target_link_libraries(testaddresseelineedit KF5::Libkdepim KF5::I18n KF5::Completion)


########### next target ###############
set(testkcheckcomboboxtest_gui_SRCS kcheckcomboboxtest_gui.cpp)

add_executable(kcheckcomboboxtest_gui ${testkcheckcomboboxtest_gui_SRCS})

target_link_libraries(kcheckcomboboxtest_gui KF5::Libkdepim KF5::I18n KF5::Completion)


########### next target ###############
set(testcompletionconfiguredialog_SRCS completionconfiguredialog_gui.cpp)

add_executable(completionconfiguredialog_gui ${testcompletionconfiguredialog_SRCS})
target_link_libraries(completionconfiguredialog_gui KF5::Libkdepim KF5::I18n KF5::Completion)

########### next target ###############
set(testldapsearchdialog_SRCS ldapsearchdialog_gui.cpp)

add_executable(ldapsearchdialog_gui ${testldapsearchdialog_SRCS})
target_link_libraries(ldapsearchdialog_gui KF5::Libkdepim KF5::I18n)
