set(newsgatorrssresource_SRCS
    soapmessages.cpp
    newsgatorjobs.cpp
    settingsdialog.cpp
    newsgatorrssresource.cpp
)

kde4_add_kcfg_files(newsgatorrssresource_SRCS newsgatorsettings.kcfgc)
kde4_add_ui_files(newsgatorrssresource_SRCS credentialswidget.ui)

kde4_add_executable(akonadi_newsgator_rss_resource RUN_UNINSTALLED ${newsgatorrssresource_SRCS})
target_link_libraries(akonadi_newsgator_rss_resource ${KDEPIMLIBS_AKONADI_LIBS} ${QT_QTDBUS_LIBRARY}
                     ${KDE4_KIO_LIBRARY} ${KDEPIMLIBS_SYNDICATION_LIBS} krssresource krss)

install(TARGETS akonadi_newsgator_rss_resource ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES newsgatorrssresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES newsgatorresource.kcfg DESTINATION ${KCFG_INSTALL_DIR})
