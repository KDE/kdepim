set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( accessiblemailwebviewtest_SRCS accessiblemailwebviewtest.cpp )

kde4_add_unit_test(
    accessiblemailwebviewtest
    ${accessiblemailwebviewtest_SRCS}
)

target_link_libraries( accessiblemailwebviewtest
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBRARY}
    ${QT_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    messageviewer
)

