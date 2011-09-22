add_subdirectory(tests)

include_directories(${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/messageviewer)
set(messagevieweraccessiblewidgetfactory_SRCS messagevieweraccessiblewidgetfactory.cpp accessiblemailwebview.cpp)
kde4_add_plugin(messagevieweraccessiblewidgetfactory ${messagevieweraccessiblewidgetfactory_SRCS})
target_link_libraries(messagevieweraccessiblewidgetfactory ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} messageviewer ${KDE4_KDEWEBKIT_LIBRARY} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS messagevieweraccessiblewidgetfactory DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/accessible)
