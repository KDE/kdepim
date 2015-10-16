set(pimcommon_translatorplugin_SRCS
   translatorplugin.cpp
   translatorview.cpp
)

add_library(pimcommon_translatorplugin MODULE ${pimcommon_translatorplugin_SRCS})

target_link_libraries(pimcommon_translatorplugin
  KF5::PimCommon
  KF5::PimTextEdit
  KF5::XmlGui
)

install(TARGETS pimcommon_translatorplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_translatorplugin pimcommon_translatorplugin.desktop)
