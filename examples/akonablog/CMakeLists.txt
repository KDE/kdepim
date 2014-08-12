set(akonablog_bin_SRCS
    main.cpp
    mainwidget.cpp
    mainwindow.cpp
    blogmodel.cpp
    microblogdelegate.cpp
    akonaditabbar.cpp
)
include_directories(
  ${Boost_INCLUDE_DIR}
)

kde4_add_app_icon(akonablog_bin_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/email.png")

add_executable(akonablog_bin ${akonablog_bin_SRCS})
set_target_properties(akonablog_bin PROPERTIES OUTPUT_NAME akonablog)

target_link_libraries(akonablog_bin KF5::AkonadiCore  Qt5::WebKitWidgets KF5::PimUtils KF5::MicroBlog)

