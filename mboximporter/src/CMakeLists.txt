configure_file(mboximporter-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/mboximporter-version.h @ONLY)

set(mboximporter_SRCS
    main.cpp
    mboxmainwindow.cpp
    mboximporterinfogui.cpp
    mboximportwidget.cpp
    mboximportkernel.cpp
    )

ki18n_wrap_ui(mboximporter_SRCS
    ui/mboximportwidget.ui
    )

add_executable(mboximporter ${mboximporter_SRCS})

target_link_libraries(mboximporter
    PRIVATE
    KF5::AkonadiCore
    KF5::IdentityManagement
    KF5::MailImporter
    KF5::MailCommon
    KF5::Completion
    KF5::I18n
    KF5::Crash
    )

install(TARGETS mboximporter  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install(PROGRAMS org.kde.mboximporter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
