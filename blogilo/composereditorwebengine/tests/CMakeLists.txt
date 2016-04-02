
set(composerhtmlSources
    main.cpp
    composerhtmleditor.cpp
    )

add_executable(composerhtmleditor ${composerhtmlSources} )
target_link_libraries(composerhtmleditor composereditorwebengineprivate 
    KF5::XmlGui
    KF5::I18n
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    )

install(TARGETS composerhtmleditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    composerhtmleditorui.rc
    DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/composerhtmleditor)

