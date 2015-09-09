
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


# Laurent install it if you want to debug it
#add_subdirectory(tests)

add_definitions(-DTRANSLATION_DOMAIN=\"libcomposereditorng\")

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
    widgets/domtreewidget.cpp 
    widgets/findreplacebar.cpp
    ${wk_HEADERS_MOC}
)

kconfig_add_kcfg_files(libcomposereditor_ng_SRCS
   globalsetting_composereditorng.kcfgc 
)


ki18n_wrap_ui(libcomposereditor_ng_SRCS ui/pagecolorbackgroundwidget.ui )
ecm_qt_declare_logging_category(libcomposereditor_ng_SRCS HEADER composereditorng_debug.h IDENTIFIER COMPOSEREDITORNG_LOG CATEGORY_NAME log_composereditorng)


add_library(composereditorng ${libcomposereditor_ng_SRCS})
generate_export_header(composereditorng BASE_NAME composereditorng)

target_link_libraries(composereditorng 
 PRIVATE
   KF5::WebKit Qt5::WebKitWidgets KF5::PimTextEdit KF5::SonnetCore pimcommon KF5::KIOWidgets KF5::XmlGui KF5::Completion Qt5::PrintSupport
)


set_target_properties( composereditorng PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION} )
install(TARGETS composereditorng ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES data/composereditorinitialhtml DESTINATION  ${KDE_INSTALL_DATADIR}/composereditor )

#install( FILES
#         composereditorng_export.h
#         composereditor.h
#         DESTINATION ${KDE_INSTALL_INCLUDEDIR}/composereditorng COMPONENT Devel )

