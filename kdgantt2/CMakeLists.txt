project(kdgantt2)

add_definitions(-DTRANSLATION_DOMAIN=\"kdgantt2\")
# Those paths need to be absolute since this file is included from other CMakeLists.txt files
set(libkdgantt2_SRCS
kdgantt_debug.cpp
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
add_library(kdgantt2 ${libkdgantt2_SRCS})
generate_export_header(kdgantt2 BASE_NAME kdgantt)
target_link_libraries(kdgantt2  PRIVATE Qt5::Widgets Qt5::PrintSupport )

target_include_directories(kdgantt2 PUBLIC "$<BUILD_INTERFACE:${kdgantt2_SOURCE_DIR};${kdgantt2_BINARY_DIR}>")

set_target_properties( kdgantt2 PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kdgantt2 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
