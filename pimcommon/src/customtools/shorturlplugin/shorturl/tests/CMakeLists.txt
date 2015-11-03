include_directories( 
   ${CMAKE_SOURCE_DIR}/pimcommon/src/customtools/shorturlplugin/
)

set(shorturlengineswigets_SRCS
   ../shorturlwidgetng.cpp
   ../shorturlconfigurewidget.cpp
   ../shorturlconfiguredialog.cpp
   ../shorturlengineplugin/shorturlenginepluginmanager.cpp
)

set(shorturlwidgetng_gui_SRCS shorturlwidgetng_gui.cpp shorturlmainwidgetng.cpp ${shorturlengineswigets_SRCS})
add_executable(shorturlwidgetng_gui ${shorturlwidgetng_gui_SRCS})
target_link_libraries(shorturlwidgetng_gui
  KF5::PimCommon KF5::Completion KF5::KIOWidgets KF5::Libkdepim KF5::XmlGui
)


