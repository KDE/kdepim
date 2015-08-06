include(ECMAddTests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories (
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_BINARY_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/templateparser
)

########### recipientseditor test ###############

set(recipienteditortest_SRCS recipientseditortest.cpp)
add_executable(recipienteditortest ${recipienteditortest_SRCS})
target_link_libraries(recipienteditortest KF5::KIOCore KF5::AkonadiContact messagecomposer kdepim)


########### recipientseditor test ###############

set(composerlineedittest_SRCS testcomposerlineedit.cpp)
add_executable(composerlineedittest ${composerlineedittest_SRCS})
ecm_mark_as_test(composerlineedittest)
target_link_libraries(composerlineedittest messagecomposer  kdepim)

