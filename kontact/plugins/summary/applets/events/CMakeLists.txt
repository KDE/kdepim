### Build applet ###

include_directories(${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces)

set(plasma_applet_events_SRCS 
    eventsapplet.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_events ${plasma_applet_events_SRCS})
target_link_libraries(plasma_applet_events ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} kdepimdbusinterfaces ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} )


install(TARGETS plasma_applet_events DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-events.desktop DESTINATION ${SERVICES_INSTALL_DIR})
