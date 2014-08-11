set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( accessiblemailwebviewtest_SRCS accessiblemailwebviewtest.cpp )

kde4_add_unit_test(
    accessiblemailwebviewtest
    ${accessiblemailwebviewtest_SRCS}
)

target_link_libraries( accessiblemailwebviewtest
    
    KF5::WebKit
    ${QT_LIBRARIES}
    Qt5::Test
    Qt5::WebKitWidgets
    messageviewer
)

