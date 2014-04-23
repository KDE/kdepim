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
foreach(it ${libkdgantt_mocable_files})
  qt4_generate_moc(${it}.h ${CMAKE_BINARY_DIR}/kdgantt2/moc_${it}.cpp)
  macro_add_file_dependencies(${it}.cpp ${CMAKE_BINARY_DIR}/kdgantt2/moc_${it}.cpp)
endforeach(it)

qt4_generate_moc(kdganttgraphicsview_p.h ${CMAKE_BINARY_DIR}/kdgantt2/moc_kdganttgraphicsview_p.cpp)
macro_add_file_dependencies(kdganttgraphicsview.cpp ${CMAKE_BINARY_DIR}/kdgantt2/moc_kdganttgraphicsview_p.cpp)
add_library(kdgantt2 ${LIBRARY_TYPE} ${libkdgantt2_SRCS})
target_link_libraries(kdgantt2  Qt5::Widgets Qt5::PrintSupport )

set_target_properties( kdgantt2 PROPERTIES VERSION 0.1.0 SOVERSION 0)
install(TARGETS kdgantt2 ${INSTALL_TARGETS_DEFAULT_ARGS} )
