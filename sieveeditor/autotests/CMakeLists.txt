
set( sieveedit_serversievesettings_source  serversievesettingstest.cpp ../serversievesettings.cpp)
kde4_add_ui_files(sieveedit_serversievesettings_source
    ../ui/serversievesettings.ui
)

kde4_add_unit_test( serversievesettingstest  ${sieveedit_serversievesettings_source})
target_link_libraries( serversievesettingstest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})


set( sieveeditorutiltest_source sieveeditorutiltest.cpp)
kde4_add_unit_test( sieveeditorutiltest ${sieveeditorutiltest_source})
target_link_libraries( sieveeditorutiltest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})
