include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories(../)


########### next target ###############
set(testspellchecklineedit_SRCS testspellchecklineedit.cpp)

add_executable(testspellchecklineedit ${testspellchecklineedit_SRCS})
ecm_mark_as_test(testspellchecklineedit)

target_link_libraries(testspellchecklineedit pimcommon )

