if (Qt5Designer_FOUND)
    kf5designerplugin_add_plugin(kdepimwidgets kdepim.widgets)

    target_link_libraries(kdepimwidgets
        KF5::Libkdepim KF5::Completion
    )
    if(NOT WIN32)
        # Since there are no libraries provided by this module,
        # there is no point including the build tree in RPath,
        # and then having to edit it at install time.
        set_target_properties(kdepimwidgets PROPERTIES
            SKIP_BUILD_RPATH TRUE
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()

    install(TARGETS
        kdepimwidgets
        DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/designer
    )
endif()

