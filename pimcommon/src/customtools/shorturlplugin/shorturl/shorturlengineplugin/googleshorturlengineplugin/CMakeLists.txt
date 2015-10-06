set(pimcommon_googleshorturlengineplugin_SRCS
    googleshorturlengineplugin.cpp
    googleshorturlengineinterface.cpp
)

add_library(pimcommon_googleshorturlengineplugin MODULE ${pimcommon_googleshorturlengineplugin_SRCS})

target_link_libraries(pimcommon_googleshorturlengineplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_googleshorturlengineplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_googleshorturlengineplugin pimcommon_googleshorturlengineplugin.desktop)

