set(pimcommon_ur1cashorturlengineplugin_SRCS
    ur1cashorturlengineplugin.cpp
    ur1cashorturlengineinterface.cpp
)

add_library(pimcommon_ur1cashorturlengineplugin MODULE ${pimcommon_ur1cashorturlengineplugin_SRCS})

target_link_libraries(pimcommon_ur1cashorturlengineplugin
  KF5::PimCommon
)

install(TARGETS pimcommon_ur1cashorturlengineplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_ur1cashorturlengineplugin pimcommon_ur1cashorturlengineplugin.desktop)

