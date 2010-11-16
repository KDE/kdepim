kde4_add_executable(sievemanager TEST sievemanager.cpp)
target_link_libraries(sievemanager ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ksieve ksieveui)
