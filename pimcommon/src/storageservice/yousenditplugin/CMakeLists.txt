set(pimcommon_yousenditplugin_SRCS
   yousenditplugin.cpp
   yousenditinterface.cpp
)

add_library(pimcommon_yousenditplugin MODULE ${pimcommon_yousenditplugin_SRCS} )

target_link_libraries(pimcommon_yousenditplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_yousenditplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_yousenditplugin pimcommon_yousenditplugin.desktop)
