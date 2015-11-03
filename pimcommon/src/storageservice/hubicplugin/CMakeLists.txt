set(pimcommon_hubicplugin_SRCS
   hubicplugin.cpp
   hubicinterface.cpp
)

add_library(pimcommon_hubicplugin MODULE ${pimcommon_hubicplugin_SRCS} )

target_link_libraries(pimcommon_hubicplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_hubicplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_hubicplugin pimcommon_hubicplugin.desktop)
