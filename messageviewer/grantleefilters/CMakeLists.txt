PROJECT(grantleefilters)

INCLUDE(${Grantlee_USE_FILE}) # This files defines the grantlee_add_plugin macro

INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} ${Grantlee_INCLUDE_DIRS} 
)

grantlee_add_plugin(grantlee_messageheaderfilters messageheadergrantleefilters FILTERS messageheaderfilter )
TARGET_LINK_LIBRARIES(grantlee_messageheaderfilters messagecore)

########### install files ###############
INSTALL(TARGETS grantlee_messageheaderfilters LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/grantlee/${Grantlee_VERSION_MAJOR}.${Grantlee_VERSION_MINOR} )

