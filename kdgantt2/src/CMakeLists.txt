add_definitions(-DTRANSLATION_DOMAIN=\"kdgantt2\")
# Those paths need to be absolute since this file is included from other CMakeLists.txt files
set(libkdgantt2_SRCS
kdganttabstractgrid.cpp
kdganttabstractrowcontroller.cpp
kdganttconstraint.cpp
kdganttconstraintgraphicsitem.cpp
kdganttconstraintmodel.cpp
kdganttconstraintproxy.cpp
kdganttdatetimegrid.cpp
kdganttforwardingproxymodel.cpp
kdganttglobal.cpp
kdganttgraphicsitem.cpp
kdganttgraphicsscene.cpp
kdganttgraphicsview.cpp
kdganttitemdelegate.cpp
kdganttlegend.cpp
kdganttlistviewrowcontroller.cpp
kdganttproxymodel.cpp
kdganttstyleoptionganttitem.cpp
kdganttsummaryhandlingproxymodel.cpp
kdgantttreeviewrowcontroller.cpp
kdganttview.cpp
)

ecm_qt_declare_logging_category(libkdgantt2_SRCS HEADER kdgantt_debug.h IDENTIFIER KDGANTT_LOG CATEGORY_NAME log_kdgantt)

add_definitions(-DKDAB_NO_UNIT_TESTS -DKDGANTT_SHAREDLIB -DKDGANTT_BUILD_KDGANTT_LIB)

# Generate moc_foo.cpp files since that's what the cpp files include (it works automatically with qmake, this way...)
# List of source files generated with "grep -l moc_ *.cpp | sed -e 's/\.cpp$//'"
# Plus kdganttgraphicsview_p
set(libkdgantt_mocable_files
  kdganttabstractgrid
  kdganttconstraintmodel
  kdganttconstraintproxy
  kdganttdatetimegrid
  kdganttforwardingproxymodel
  kdganttgraphicsscene
  kdganttgraphicsview
  kdganttitemdelegate
  kdganttproxymodel
  kdganttsummaryhandlingproxymodel
  kdganttview
  kdganttlegend
  kdgantttimescalezoomdialog
)

add_library(KF5KDGantt2 ${libkdgantt2_SRCS})
generate_export_header(KF5KDGantt2 BASE_NAME kdgantt)
add_library(KF5::KDGantt2 ALIAS KF5KDGantt2)
target_link_libraries(KF5KDGantt2  PRIVATE Qt5::Widgets Qt5::PrintSupport )

target_include_directories(KF5KDGantt2 INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KDGantt2/;${KDE_INSTALL_INCLUDEDIR_KF5}/kdgantt2>")
target_include_directories(KF5KDGantt2 PUBLIC "$<BUILD_INTERFACE:${kdgantt2_SOURCE_DIR};${kdgantt2_BINARY_DIR}>")


set_target_properties(KF5KDGantt2 PROPERTIES
    VERSION ${KDGANTT2_VERSION_STRING}
    SOVERSION ${KDGANTT2_SOVERSION}
    EXPORT_NAME KDGantt2
)

install(TARGETS
    KF5KDGantt2
    EXPORT KF5KDGantt2Targets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(kdgantt2_CamelCase_HEADERS
  HEADER_NAMES
  KDGanttItemDelegate
  KDGanttGlobal
  KDGanttGraphicsItem
  KDGanttGraphicsView
  KDGanttAbstractRowController
  KDGanttView
  KDGanttDateTimeGrid
  KDGanttItemDelegate
  KDGanttStyleOptionGanttItem
  KDGanttAbstractGrid
  KDGanttConstraint
  REQUIRED_HEADERS kdgantt2_HEADERS
  PREFIX KDGantt2
)

ecm_generate_pri_file(BASE_NAME KDGantt2
    LIB_NAME KF5KDGantt2
    DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KDGantt2
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${kdgantt2_SOURCE_DIR})
install(FILES
    ${kdgantt2_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KDGantt2
    COMPONENT Devel
)

install(FILES
    ${kdgantt2_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kdgantt_export.h
    ${kdgantt2_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/kdgantt2
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

endif()
