set(opmlrssresource_SRCS
    opmljobs.cpp
    opmlrssresource.cpp
)

kde4_add_kcfg_files(opmlrssresource_SRCS opmlsettings.kcfgc)

kde4_add_executable(akonadi_opml_rss_resource RUN_UNINSTALLED ${opmlrssresource_SRCS})
target_link_libraries(akonadi_opml_rss_resource ${KDEPIMLIBS_AKONADI_LIBS} ${QT_QTDBUS_LIBRARY}
                     ${KDE4_KIO_LIBRARY} ${KDEPIMLIBS_SYNDICATION_LIBS} krssresource krss)

install(TARGETS akonadi_opml_rss_resource ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES opmlrssresource.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES opmlresource.kcfg DESTINATION ${KCFG_INSTALL_DIR})
