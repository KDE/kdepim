include_directories( 
   ${CMAKE_SOURCE_DIR}/pimcommon/src/customtools/shorturlplugin/
)

set(shorturlengineswigets_SRCS
   ../abstractshorturl.cpp
   ../shorturlwidget.cpp
   ../engine/tinyurlshorturl.cpp
   ../engine/googleshorturl.cpp
   ../shorturlconfigurewidget.cpp
   ../shorturlutils.cpp
   ../shorturlconfiguredialog.cpp
   ../engine/migremeshorturl.cpp
   ../engine/triopabshorturl.cpp
   ../engine/ur1cashorturl.cpp
   ../engine/isgdshorturl.cpp
)

set(shorturlwidget_gui_SRCS shorturlwidget_gui.cpp shorturlmainwidget.cpp ${shorturlengineswigets_SRCS})
add_executable(shorturlwidget_gui ${shorturlwidget_gui_SRCS})
target_link_libraries(shorturlwidget_gui
  KF5::PimCommon KF5::Completion KF5::KIOWidgets KF5::Libkdepim KF5::XmlGui
)

set(shorturlwidgetng_gui_SRCS shorturlwidgetng_gui.cpp shorturlmainwidgetng.cpp ${shorturlengineswigets_SRCS})
add_executable(shorturlwidgetng_gui ${shorturlwidgetng_gui_SRCS})
target_link_libraries(shorturlwidgetng_gui
  KF5::PimCommon KF5::Completion KF5::KIOWidgets KF5::Libkdepim KF5::XmlGui
)


