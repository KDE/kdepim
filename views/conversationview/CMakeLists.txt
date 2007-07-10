project(conversationview)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

include_directories (${KDE4_INCLUDES})
add_definitions (${KDE4_DEFINITIONS} ${QT_DEFINITIONS})
link_directories (${KDE4_LIBRARIES})

set ( conversationSources 
 main.cpp
 message.cpp
 mailview.cpp
 foldermodel.cpp
 dummykonadiadapter.cpp
 conversationdelegate.cpp
 conversation.cpp
 conversationview.cpp
 conversationwidget.cpp
 searchline.cpp
 folderproxymodel.cpp
 mailvieweventfilter.cpp
# maildisplay.cpp
)

set ( tmpSRC 
 tmp.cpp
 maildisplay.cpp
 mailvieweventfilter.cpp
)

kde4_add_executable(conversationview ${conversationSources})
target_link_libraries (conversationview ${KDE4_KDEUI_LIBS})

kde4_add_executable(tmp ${tmpSRC})
target_link_libraries (tmp ${KDE4_KDEUI_LIBS})

