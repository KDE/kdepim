set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( accessiblemailwebviewtest_SRCS accessiblemailwebviewtest.cpp )

add_executable(accessiblemailwebviewtest ${accessiblemailwebviewtest_SRCS})
add_test(accessiblemailwebviewtest accessiblemailwebviewtest)
ecm_mark_as_test(accessiblemailwebviewtest)
target_link_libraries( accessiblemailwebviewtest
    KF5::WebKit
    Qt5::Test
    Qt5::WebKitWidgets
    messageviewer
)

