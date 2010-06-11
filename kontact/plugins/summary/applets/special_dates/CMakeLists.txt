### Build applet ###
set(plasma_applet_special_dates_SRCS 
    specialdatesapplet.cpp
    specialdatewidget.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_special_dates ${plasma_applet_special_dates_SRCS})
target_link_libraries(plasma_applet_special_dates ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_special_dates DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-special_dates.desktop DESTINATION ${SERVICES_INSTALL_DIR})