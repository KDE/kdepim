add_definitions(-DTRANSLATION_DOMAIN=\"libcomposereditorwebengine\")

set(libcomposereditor_webengine_SRCS
    composerwebengine.cpp
    private/composerwebengine_p.cpp
    table/composertabledialog.cpp
    link/composeranchordialog.cpp
    link/composerlinkdialog.cpp
    utils/composereditorutils.cpp
    image/composerimagedialog.cpp
    image/composerimageresizetooltip.cpp
    #image/composerimageresizewidget.cpp
    widgets/findreplacebar.cpp
    composerwebenginewidget.cpp
    )

ecm_qt_declare_logging_category(libcomposereditor_webengine_SRCS HEADER composereditorwebengine_debug.h IDENTIFIER COMPOSEREDITORWEBENGINE_LOG CATEGORY_NAME log_composereditorwebengine)


kconfig_add_kcfg_files(libcomposereditor_webengine_SRCS
    globalsetting_composereditorwebengine.kcfgc
    )


add_library(composereditorwebengineprivate ${libcomposereditor_webengine_SRCS})
generate_export_header(composereditorwebengineprivate BASE_NAME composereditorwebengine)

target_link_libraries(composereditorwebengineprivate
    PRIVATE
    KF5::PimTextEdit
    KF5::SonnetCore
    KF5::PimCommon
    KF5::KIOWidgets
    KF5::XmlGui
    KF5::Completion
    Qt5::WebEngine
    Qt5::WebEngineWidgets
    KF5::WebEngineViewer
    KF5::I18n
    KF5::SonnetUi
    )

target_include_directories(composereditorwebengineprivate PUBLIC "$<BUILD_INTERFACE:${composereditorwebengine_SOURCE_DIR}/src;${composereditorwebengine_BINARY_DIR}/src>")


set_target_properties(composereditorwebengineprivate  PROPERTIES 
    VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS composereditorwebengineprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

install( FILES data/composereditorinitialhtml DESTINATION  ${KDE_INSTALL_DATADIR}/composereditorwebengine )


