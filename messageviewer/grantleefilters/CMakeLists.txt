project(grantleefilters)


add_library(grantlee_messageheaderfilters MODULE messageheadergrantleefilters messageheaderfilter)
GRANTLEE_ADJUST_PLUGIN_NAME(grantlee_messageheaderfilters)

TARGET_LINK_LIBRARIES(grantlee_messageheaderfilters messagecore Grantlee5::TextDocument Grantlee5::Templates)

install(TARGETS grantlee_messageheaderfilters LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}/grantlee/${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR} )

