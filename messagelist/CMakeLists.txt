project(messagelist)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
    ${Boost_INCLUDE_DIRS}
)

set(libmessagelist_SRCS
    core/aggregation.cpp
    core/aggregationcombobox.cpp
    core/aggregationconfigbutton.cpp
    core/aggregationeditor.cpp
    core/comboboxutils.cpp
    core/configureaggregationsdialog.cpp
    core/configurethemesdialog.cpp
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
    core/optionseteditor.cpp
    core/theme.cpp
    core/themedelegate.cpp
    core/themecombobox.cpp
    core/themeconfigbutton.cpp
    core/themeeditor.cpp
    core/storagemodelbase.cpp
    core/sortorder.cpp
    core/view.cpp
    core/widgetbase.cpp

    pane.cpp
    storagemodel.cpp
    widget.cpp
)

set(libmessagelist_HEADERS
    pane.h
    storagemodel.h
    widget.h
)

# kde4_add_kcfg_files(libmessagelist_SRCS
#     globalsettings_base.kcfgc
# )

kde4_add_library(messagelist SHARED ${libmessagelist_SRCS})
target_link_libraries(messagelist
    ${KDE4_KDEUI_LIBS}
    ${KDE4_AKONADI_LIBS}
    ${KDE4_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    kdepim
)


install(TARGETS messagelist ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ${libmessagelist_HEADERS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist
         COMPONENT devel)

