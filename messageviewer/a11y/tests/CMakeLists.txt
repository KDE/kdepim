set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${QT_INCLUDES}
${CMAKE_SOURCE_DIR}/messageviewer)

set( accessiblemailwebviewtest_SRCS accessiblemailwebviewtest.cpp )

kde4_add_unit_test( 
  accessiblemailwebviewtest
  ${accessiblemailwebviewtest_SRCS}
)

target_link_libraries( accessiblemailwebviewtest
  ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} messageviewer
  ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  )

