include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories(../)

########### next target ###############

ecm_add_tests(testaddresseelineedit.cpp kcheckcomboboxtest_gui.cpp testldapclient.cpp testspellchecklineedit.cpp NAME_PREFIX "libkdepim-" LINK_LIBRARIES kdepim Qt5::Test)


########### next target ###############
set(testcompletionconfiguredialog_SRCS completionconfiguredialog_gui.cpp)

kde4_add_executable(completionconfiguredialog_gui TEST ${testcompletionconfiguredialog_SRCS})

target_link_libraries(completionconfiguredialog_gui kdepim ${KDE4_KDEUI_LIBS})
