project( contactprintthemeeditor )


set(contactprintthemeeditor_SRCS
    main.cpp
    defaultcompletion.cpp
    editorpage.cpp
    contactprintthemeconfiguredialog.cpp
    themedefaulttemplate.cpp
    themeeditormainwindow.cpp
    themeeditorpage.cpp
    contactprintthemeeditorutil.cpp
    themeeditorwidget.cpp
    themetemplatewidget.cpp
    contactprintthemepreview.cpp
    )
if (QTWEBENGINE_SUPPORT_OPTION)
    set(contactprintthemeeditor_web_LIB Qt5::WebEngineWidgets)
else()
    set(contactprintthemeeditor_web_LIB Qt5::WebKitWidgets Qt5::WebKit)
endif()


ecm_qt_declare_logging_category(contactprintthemeeditor_SRCS HEADER contactprintthemeeditor_debug.h IDENTIFIER CONTACTPRINTTHEMEEDITOR_LOG CATEGORY_NAME log_contactprintthemeeditor)
qt5_add_resources(contactprintthemeeditor_SRCS contactprintthemeeditor.qrc)

add_executable(contactprintthemeeditor ${contactprintthemeeditor_SRCS} )
target_link_libraries(contactprintthemeeditor
    ${contactprintthemeeditor_web_LIB}
    KF5::DBusAddons
    KF5::NewStuff
    KF5::AkonadiContact
    KF5::PimCommon
    grantleethemeeditor
    KF5::KaddressbookGrantlee
    KF5::PimTextEdit
    KF5::I18n
    )

install(TARGETS contactprintthemeeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.contactprintthemeeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
