set(pimcommon_dropboxplugin_SRCS
   dropboxplugin.cpp
   dropboxinterface.cpp
)

add_library(pimcommon_dropboxplugin MODULE ${pimcommon_dropboxplugin_SRCS} )

target_link_libraries(pimcommon_dropboxplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_dropboxplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_dropboxplugin pimcommon_dropboxplugin.desktop)
