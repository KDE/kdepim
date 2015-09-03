set(pimcommon_shorturlplugin_SRCS
   shorturlplugin.cpp
   shorturlview.cpp
)

add_library(pimcommon_shorturlplugin MODULE ${pimcommon_shorturlplugin_SRCS})

target_link_libraries(pimcommon_shorturlplugin
  pimcommon
)

install(TARGETS pimcommon_shorturlplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_shorturlplugin pimcommon_shorturlplugin.desktop)
