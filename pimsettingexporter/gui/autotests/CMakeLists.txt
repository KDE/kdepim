set( pimsettingexporter_kcfg_SRCS ../settings/pimsettingexporterglobalconfig.kcfgc )

kconfig_add_kcfg_files(pimsettingexporter_kcfg_SRCS ${pimsettingexporter_kcfg_SRCS})

add_executable( synchronizeresourcedialogtest synchronizeresourcedialogtest.cpp ../dialog/synchronizeresourcedialog.cpp)
add_test(synchronizeresourcedialogtest synchronizeresourcedialogtest)
ecm_mark_as_test(synchronizeresourcedialogtest)
target_link_libraries(synchronizeresourcedialogtest Qt5::Test Qt5::Widgets KF5::I18n KF5::ItemViews KF5::ConfigCore)

add_executable( pimsettingexporterconfiguredialogtest pimsettingexporterconfiguredialogtest.cpp ../dialog/pimsettingexporterconfiguredialog.cpp ../widgets/pimsettingexporterconfigurewidget.cpp ${pimsettingexporter_kcfg_SRCS})
add_test(pimsettingexporterconfiguredialogtest pimsettingexporterconfiguredialogtest)
ecm_mark_as_test(pimsettingexporterconfiguredialogtest)
target_link_libraries(pimsettingexporterconfiguredialogtest Qt5::Test Qt5::Widgets KF5::I18n KF5::ItemViews KF5::ConfigCore KF5::ConfigGui)


add_executable( pimsettingexporterconfigurewidgettest pimsettingexporterconfigurewidgettest.cpp ../widgets/pimsettingexporterconfigurewidget.cpp ${pimsettingexporter_kcfg_SRCS})
add_test(pimsettingexporterconfigurewidgettest pimsettingexporterconfigurewidgettest)
ecm_mark_as_test(pimsettingexporterconfigurewidgettest)
target_link_libraries(pimsettingexporterconfigurewidgettest Qt5::Test Qt5::Widgets KF5::I18n KF5::ItemViews KF5::ConfigCore KF5::ConfigGui)
