set(rssfilteringagent_SRCS
    filterdata.cpp
    filterassistant.cpp
    checkablefeedlistmodel.cpp
    virtualfeedlistmodel.cpp
    createvirtualfeedjob.cpp
    rssdata.cpp
    factories.cpp
    rssfilteringagent.cpp
    linkcommandeditor.cpp
    filterlisteditor.cpp
)

kde4_add_ui_files( rssfilteringagent_SRCS
                   linkcommandeditor.ui
                   filterpropertieswidget.ui
                   filterlistwidget.ui
)

kde4_add_executable(akonadi_rss_filtering_agent RUN_UNINSTALLED ${rssfilteringagent_SRCS})
target_link_libraries(akonadi_rss_filtering_agent ${KDEPIMLIBS_AKONADI_LIBS} krss akonadi-filter
                      akonadi-filter-ui)

install(TARGETS akonadi_rss_filtering_agent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES rssfilteringagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
