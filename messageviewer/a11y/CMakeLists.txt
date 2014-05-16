
include_directories( ../ )

set(messagevieweraccessiblewidgetfactory_SRCS
    messagevieweraccessiblewidgetfactory.cpp
    accessiblemailwebview.cpp
    )

add_library(messagevieweraccessiblewidgetfactory
 MODULE 
    ${messagevieweraccessiblewidgetfactory_SRCS}
)

target_link_libraries(messagevieweraccessiblewidgetfactory
    messageviewer
    KF5::WebKit
    Qt5::WebKitWidgets
)

install(TARGETS messagevieweraccessiblewidgetfactory DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/accessible)

add_subdirectory(tests)
