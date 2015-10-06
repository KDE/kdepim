set(pimcommon_isgdshorturlengineplugin_SRCS
    isgdshorturlengineplugin.cpp
    isgdshorturlengineinterface.cpp
)

add_library(pimcommon_isgdshorturlengineplugin MODULE ${pimcommon_isgdshorturlengineplugin_SRCS})

target_link_libraries(pimcommon_isgdshorturlengineplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_isgdshorturlengineplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_isgdshorturlengineplugin pimcommon_isgdshorturlengineplugin.desktop)

