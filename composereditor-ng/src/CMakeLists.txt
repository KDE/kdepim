
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
)

kconfig_add_kcfg_files(libcomposereditor_ng_SRCS
   globalsetting_composereditorng.kcfgc 
)


ki18n_wrap_ui(libcomposereditor_ng_SRCS ui/pagecolorbackgroundwidget.ui )
ecm_qt_declare_logging_category(libcomposereditor_ng_SRCS HEADER composereditorng_debug.h IDENTIFIER COMPOSEREDITORNG_LOG CATEGORY_NAME log_composereditorng)


add_library(KF5ComposerEditorNG ${libcomposereditor_ng_SRCS})
generate_export_header(KF5ComposerEditorNG BASE_NAME composereditorng)
add_library(KF5::ComposerEditorNG ALIAS KF5ComposerEditorNG)

target_link_libraries(KF5ComposerEditorNG
 PRIVATE
   KF5::WebKit Qt5::WebKitWidgets KF5::PimTextEdit KF5::SonnetCore KF5::PimCommon KF5::KIOWidgets KF5::XmlGui KF5::Completion Qt5::PrintSupport
)

target_include_directories(KF5ComposerEditorNG INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/ComposerEditorNG/;${KDE_INSTALL_INCLUDEDIR_KF5}/composereditorng>")
target_include_directories(KF5ComposerEditorNG PUBLIC "$<BUILD_INTERFACE:${composereditorng_SOURCE_DIR}/src;${composereditorng_BINARY_DIR}/src>")


set_target_properties(KF5ComposerEditorNG PROPERTIES
    VERSION ${COMPOSEREDITORNG_VERSION_STRING}
    SOVERSION ${COMPOSEREDITORNG_SOVERSION}
    EXPORT_NAME ComposerEditorNG
)

install(TARGETS
    KF5ComposerEditorNG
    EXPORT KF5ComposerEditorNGTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(composereditorng_CamelCase_HEADERS
  HEADER_NAMES
  ComposerEditor
  ComposerView
  REQUIRED_HEADERS composereditorng_HEADERS
  PREFIX ComposerEditorNG
)


ecm_generate_pri_file(BASE_NAME ComposerEditorNG
    LIB_NAME KF5ComposerEditorNG
    DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/ComposerEditorNG
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${composereditorng_SOURCE_DIR})
install(FILES
    ${composereditorng_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/ComposerEditorNG
    COMPONENT Devel
)

install(FILES
    ${composereditorng_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/composereditorng_export.h
    ${composereditorng_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/composereditorng
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})


endif()


install( FILES data/composereditorinitialhtml DESTINATION  ${KDE_INSTALL_DATADIR}/composereditor )


