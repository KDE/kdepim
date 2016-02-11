set(newdesigntest_SRCS
    main.cpp
    )

add_executable(newdesigntest ${newdesigntest_SRCS})

target_link_libraries(newdesigntest
    Qt5::Widgets
    KF5::Contacts
    KF5::I18n
    KF5::Completion
    KF5::IconThemes
    KF5::WidgetsAddons
    )

