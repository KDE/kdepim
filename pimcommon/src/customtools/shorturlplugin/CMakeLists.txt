set(pimcommon_shorturlplugin_SRCS
   shorturlplugin.cpp
   shorturlview.cpp
)

set(pimcommon_shorturlplugin_shorturllib_SRCS
   shorturl/abstractshorturl.cpp
   shorturl/shorturlwidget.cpp
   shorturl/engine/tinyurlshorturl.cpp
   shorturl/engine/googleshorturl.cpp
   shorturl/shorturlconfigurewidget.cpp
   shorturl/shorturlutils.cpp
   shorturl/shorturlconfiguredialog.cpp
   shorturl/engine/migremeshorturl.cpp
   shorturl/engine/triopabshorturl.cpp
   shorturl/engine/ur1cashorturl.cpp
   shorturl/engine/isgdshorturl.cpp
)

add_library(pimcommon_shorturlplugin MODULE ${pimcommon_shorturlplugin_SRCS} ${pimcommon_shorturlplugin_shorturllib_SRCS})

target_link_libraries(pimcommon_shorturlplugin
  KF5::PimCommon KF5::XmlGui KF5::Completion KF5::KIOWidgets KF5::Libkdepim
)

install(TARGETS pimcommon_shorturlplugin DESTINATION ${PLUGIN_INSTALL_DIR}/pimcommon)
kcoreaddons_desktop_to_json(pimcommon_shorturlplugin pimcommon_shorturlplugin.desktop)

if (BUILD_TESTING)
   add_subdirectory(shorturl/tests)
   add_subdirectory(shorturl/autotests)
endif()
