project(messagelist)

kde_enable_exceptions()
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

option(KDEPIM_FOLDEROPEN_PROFILE "Enable debug output for timing folder loading" FALSE)
configure_file(config-messagelist.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messagelist.h)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${BALOO_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/messagecore
  )


add_definitions(-DTRANSLATION_DOMAIN=\"libmessagelist\")

add_subdirectory(pics)
add_subdirectory(tests)

set(libmessagelist_SRCS
  core/aggregation.cpp
  core/delegate.cpp
  core/filter.cpp
  core/item.cpp
  core/groupheaderitem.cpp
  core/manager.cpp
  core/messageitem.cpp
  core/messageitemsetmanager.cpp
  core/model.cpp
  core/modelinvariantindex.cpp
  core/modelinvariantrowmapper.cpp
  core/optionset.cpp
  core/theme.cpp
  core/themedelegate.cpp
  core/storagemodelbase.cpp
  core/sortorder.cpp
  core/view.cpp
  core/widgetbase.cpp
  core/quicksearchline.cpp

  utils/aggregationcombobox.cpp
  utils/aggregationconfigbutton.cpp
  utils/aggregationeditor.cpp
  utils/comboboxutils.cpp
  utils/configureaggregationsdialog.cpp
  utils/configurethemesdialog.cpp
  utils/optionseteditor.cpp
  utils/themecombobox.cpp
  utils/themeconfigbutton.cpp
  utils/themeeditor.cpp

  pane.cpp
  storagemodel.cpp
  widget.cpp
  messagelistutil.cpp
)

set(libmessagelist_core_HEADERS
  core/enums.h
  core/item.h
  core/messageitem.h
  core/model.h
  core/modelinvariantindex.h
  core/modelinvariantrowmapper.h
  ${CMAKE_CURRENT_BINARY_DIR}/core/settings.h
  core/storagemodelbase.h
  core/view.h
  core/widgetbase.h
)

set(libmessagelist_utils_HEADERS
  utils/aggregationcombobox.h
  utils/aggregationconfigbutton.h
  utils/configureaggregationsdialog.h
  utils/configurethemesdialog.h
  utils/themecombobox.h
  utils/themeconfigbutton.h
)

set(libmessagelist_HEADERS
  pane.h
  storagemodel.h
  widget.h
)

kconfig_add_kcfg_files(libmessagelist_SRCS USE_RELATIVE_PATH
  core/settings.kcfgc
)

add_library(messagelist ${LIBRARY_TYPE} ${libmessagelist_SRCS})
target_link_libraries(messagelist
  
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  baloopim
  messagecore
)

set_target_properties(messagelist PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS messagelist ${INSTALL_TARGETS_DEFAULT_ARGS})

#install( FILES ${libmessagelist_HEADERS}
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist
#         COMPONENT devel)
#install( FILES ${libmessagelist_core_HEADERS}
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist/core
#         COMPONENT devel)
#install( FILES ${libmessagelist_utils_HEADERS}
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist/utils
#         COMPONENT devel)

