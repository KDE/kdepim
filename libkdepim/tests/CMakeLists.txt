include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories(../)


########### next target ###############
set(testldapclient_SRCS testldapclient.cpp)

add_executable(testldapclient ${testldapclient_SRCS})
ecm_mark_as_test(testldapclient)

target_link_libraries(testldapclient kdepim )


########### next target ###############
set(testaddresseelineedit_SRCS testaddresseelineedit.cpp)

add_executable(testaddresseelineedit ${testaddresseelineedit_SRCS})
ecm_mark_as_test(kcheckcomboboxtest_gui)

target_link_libraries(testaddresseelineedit kdepim )


########### next target ###############
set(testkcheckcomboboxtest_gui_SRCS kcheckcomboboxtest_gui.cpp)

add_executable(kcheckcomboboxtest_gui ${testkcheckcomboboxtest_gui_SRCS})
ecm_mark_as_test(kcheckcomboboxtest_gui)

target_link_libraries(kcheckcomboboxtest_gui kdepim )


########### next target ###############
set(testcompletionconfiguredialog_SRCS completionconfiguredialog_gui.cpp)

add_executable(completionconfiguredialog_gui ${testcompletionconfiguredialog_SRCS})
ecm_mark_as_test(completionconfiguredialog_gui)

target_link_libraries(completionconfiguredialog_gui kdepim )


