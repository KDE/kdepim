kde4_no_enable_final(messagelist)
project(messagelist)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")

option(KDEPIM_FOLDEROPEN_PROFILE "Enable debug output for timing folder loading" FALSE)
configure_file(config-messagelist.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-messagelist.h)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
  ${BALOO_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/messagecore
  )


add_subdirectory(pics)
add_subdirectory(tests)

set(libmessagelist_SRCS
  core/aggregation.cpp
  core/callbacknepomukresourceretriever.cpp
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

soprano_add_ontology(libmessagelist_SRCS
  ${CMAKE_SOURCE_DIR}/messagecore/ontology/messagetag.trig
  "MessageTag"
  "Vocabulary"
  "trig"
)

kde4_add_kcfg_files(libmessagelist_SRCS USE_RELATIVE_PATH
  core/settings.kcfgc
)

kde4_add_library(messagelist ${LIBRARY_TYPE} ${libmessagelist_SRCS})
target_link_libraries(messagelist
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${SOPRANO_LIBRARIES}
  ${NEPOMUK_CORE_LIBRARY}
  ${NEPOMUK_QUERY_LIBRARIES}
  ${BALOO_LIBRARIES}
  messagecore
  niefast_apps
)

set_target_properties(messagelist PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
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

