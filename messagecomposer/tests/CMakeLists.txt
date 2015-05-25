include(ECMAddTests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories (
    ${Boost_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_BINARY_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/templateparser
)

set(testkmeditor_SRCS testkmeditor.cpp )

add_executable(testkmeditor ${testkmeditor_SRCS})

target_link_libraries(testkmeditor messagecomposer  KF5::PimTextEdit )

########### next target ###############

set(testkmeditorwin_SRCS testkmeditorwin.cpp )

add_executable(testkmeditorwin ${testkmeditorwin_SRCS})

target_link_libraries(testkmeditorwin messagecomposer  KF5::PimTextEdit )


########### recipientseditor test ###############

set(recipienteditortest_SRCS recipientseditortest.cpp)
add_executable(recipienteditortest ${recipienteditortest_SRCS})
target_link_libraries(recipienteditortest KF5::KIOCore KF5::AkonadiContact messagecomposer kdepim)


########### recipientseditor test ###############

set(composerlineedittest_SRCS testcomposerlineedit.cpp)
add_executable(composerlineedittest ${composerlineedittest_SRCS})
ecm_mark_as_test(composerlineedittest)
target_link_libraries(composerlineedittest messagecomposer  kdepim)

