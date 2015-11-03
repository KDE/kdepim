set(pimcommon_tinyplugin_SRCS
    tinyurlengineplugin.cpp
    tinyurlengineinterface.cpp
)

add_library(pimcommon_tinyurlengineplugin MODULE ${pimcommon_tinyplugin_SRCS})

target_link_libraries(pimcommon_tinyurlengineplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_tinyurlengineplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_tinyurlengineplugin pimcommon_tinyurlengineplugin.desktop)

