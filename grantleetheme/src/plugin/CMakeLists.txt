kde_enable_exceptions()

set(grantleeplugin_SRCS
    kdegrantleeplugin.cpp
    icon.cpp
)

add_library(kde_grantlee_plugin MODULE ${grantleeplugin_SRCS})
grantlee_adjust_plugin_name(kde_grantlee_plugin)
target_link_libraries(kde_grantlee_plugin
    Grantlee5::Templates
    KF5::IconThemes
)

install(TARGETS kde_grantlee_plugin
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR}/
)
