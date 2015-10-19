set(pimcommon_webdavplugin_SRCS
   webdavplugin.cpp
   webdavinterface.cpp
)

add_library(pimcommon_webdavplugin MODULE ${pimcommon_webdavplugin_SRCS} )

target_link_libraries(pimcommon_webdavplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_webdavplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_webdavplugin pimcommon_webdavplugin.desktop)
