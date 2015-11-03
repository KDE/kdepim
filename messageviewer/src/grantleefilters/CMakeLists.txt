#project(grantleefilters)


add_library(grantlee_messageheaderfilters MODULE messageheadergrantleefilters messageheaderfilter)
GRANTLEE_ADJUST_PLUGIN_NAME(grantlee_messageheaderfilters)

target_link_libraries(grantlee_messageheaderfilters KF5::MessageCore Grantlee5::TextDocument Grantlee5::Templates)

install(TARGETS grantlee_messageheaderfilters LIBRARY DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR} )

