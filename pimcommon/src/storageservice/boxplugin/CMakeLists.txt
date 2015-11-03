set(pimcommon_boxplugin_SRCS
   boxplugin.cpp
   boxinterface.cpp
)

add_library(pimcommon_boxplugin MODULE ${pimcommon_boxplugin_SRCS} )

target_link_libraries(pimcommon_boxplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_boxplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_boxplugin pimcommon_boxplugin.desktop)
