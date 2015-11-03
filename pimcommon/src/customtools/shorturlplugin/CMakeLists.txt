set(pimcommon_shorturlplugin_SRCS
   shorturlplugin.cpp
   shorturlview.cpp
)

set(pimcommon_shorturlplugin_shorturllib_SRCS
   shorturl/shorturlwidgetng.cpp
   shorturl/shorturlconfigurewidget.cpp
   shorturl/shorturlconfiguredialog.cpp
   shorturl/shorturlengineplugin/shorturlenginepluginmanager.cpp
)

add_library(pimcommon_shorturlplugin MODULE ${pimcommon_shorturlplugin_SRCS} ${pimcommon_shorturlplugin_shorturllib_SRCS})

target_link_libraries(pimcommon_shorturlplugin
  KF5::PimCommon KF5::XmlGui KF5::Completion KF5::KIOWidgets KF5::Libkdepim
)

install(TARGETS pimcommon_shorturlplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_shorturlplugin pimcommon_shorturlplugin.desktop)

add_subdirectory(shorturl)
