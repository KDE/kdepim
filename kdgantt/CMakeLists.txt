project(kdgantt)

# Those paths need to be absolute since this variable is used in other CMakeLists.txt files
set(libkdgantt_SRCS
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttglobal.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttview.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttstyleoptionganttitem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttgraphicsview.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttabstractrowcontroller.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdgantttreeviewrowcontroller.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttlistviewrowcontroller.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttgraphicsscene.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttgraphicsitem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttconstraint.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttconstraintproxy.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttconstraintgraphicsitem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttitemdelegate.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttforwardingproxymodel.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttsummaryhandlingproxymodel.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttproxymodel.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttconstraintmodel.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttabstractgrid.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt/kdganttdatetimegrid.cpp
)
