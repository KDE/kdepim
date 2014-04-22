kde_enable_exceptions()

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


# Laurent install it if you want to debug it
#add_subdirectory(tests)

add_definitions(-DQT_STATICPLUGIN)
#qt4_wrap_cpp(wk_HEADERS_MOC spellplugin/qwebkitplatformplugin.h)

#generate_export_header(COMPOSEREDITORNG BASE_NAME composereditor)
find_package(KF5WebKit ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Sonnet ${KF5_VERSION} CONFIG REQUIRED)

set(libcomposereditor_ng_SRCS 
    composereditor.cpp
    composerview.cpp
    link/composerlinkdialog.cpp
    link/composeranchordialog.cpp
    private/composerview_p.cpp
    pagecolor/pagecolorbackgrounddialog.cpp
    pagecolor/pagecolorbackgroundwidget.cpp
    extendattributes/extendattributesdialog.cpp
    extendattributes/extendattributesbutton.cpp
    extendattributes/extendattributeswidget.cpp
    extendattributes/extendattributesutils.cpp
    utils/composereditorutils.cpp
    image/composerimagedialog.cpp
    image/composerimageresizewidget.cpp
    table/composertabledialog.cpp
    table/composertableresizewidget.cpp
    table/composertableformatdialog.cpp
    table/composertablecellformatdialog.cpp
    table/composertableactionmenu.cpp
    table/composercellsizewidget.cpp
    image/composerimageresizetooltip.cpp
    list/composerlistdialog.cpp
    helper/tablehelper.cpp
    helper/listhelper.cpp
    spellplugin/kspellplugin.cpp 
    widgets/domtreewidget.cpp 
    widgets/findreplacebar.cpp
    ${wk_HEADERS_MOC}
)

kconfig_add_kcfg_files(libcomposereditor_ng_SRCS
   globalsettings_base.kcfgc 
)


qt5_wrap_ui(libcomposereditor_ng_SRCS ui/pagecolorbackgroundwidget.ui )


add_library(composereditorng ${LIBRARY_TYPE} ${libcomposereditor_ng_SRCS})

target_link_libraries(composereditorng 
   ${KDE4_KDEWEBKIT_LIBRARY} KF5::WebKit ${QT_QTWEBKIT_LIBRARY} KF5::PimTextEdit ${KDE4_KPRINTUTILS_LIBS} KF5::KDELibs4Support KF5::SonnetCore
)


set_target_properties( composereditorng PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS composereditorng ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES data/composereditorinitialhtml DESTINATION  ${DATA_INSTALL_DIR}/composereditor )

#install( FILES
#         composereditor_export.h
#         composereditor.h
#         DESTINATION ${INCLUDE_INSTALL_DIR}/composereditorng COMPONENT Devel )

