include_directories(${kaddressbook_SOURCE_DIR})

set(kontact_kaddressbookplugin_PART_SRCS
    kaddressbook_plugin.cpp
)
ecm_qt_declare_logging_category(kontact_kaddressbookplugin_PART_SRCS HEADER kaddressbookplugin_debug.h IDENTIFIER KADDRESSBOOKPLUGIN_LOG CATEGORY_NAME log_kaddressbook_plugin)

qt5_add_dbus_interfaces(kontact_kaddressbookplugin_PART_SRCS ../org.kde.kaddressbook.xml)


add_library(kontact_kaddressbookplugin MODULE ${kontact_kaddressbookplugin_PART_SRCS})
target_link_libraries(kontact_kaddressbookplugin
    KF5::KontactInterface
)

install(TARGETS kontact_kaddressbookplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kaddressbookplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
install(FILES kaddressbook.setdlg DESTINATION ${KDE_INSTALL_DATADIR}/kontact/ksettingsdialog)
