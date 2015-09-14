if (Qt5Designer_FOUND)
    kf5designerplugin_add_plugin(mailcommonwidgets mailcommon.widgets)

    target_link_libraries(mailcommonwidgets
        KF5::MailCommon
        KF5::Libkdepim
    )
    if(NOT WIN32)
        # Since there are no libraries provided by this module,
        # there is no point including the build tree in RPath,
        # and then having to edit it at install time.
        set_target_properties(mailcommonwidgets PROPERTIES
            SKIP_BUILD_RPATH TRUE
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()

    install(TARGETS
        mailcommonwidgets
        DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/designer
    )
endif()

