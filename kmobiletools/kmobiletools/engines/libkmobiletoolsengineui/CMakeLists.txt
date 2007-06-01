

set(kmobiletoolsengineui_LIB_SRCS 
   connectionsPage.cpp
   scanprogressPage.cpp
   devicesfoundPage.cpp
   )

kde4_automoc(${kmobiletoolsengineui_LIB_SRCS})

kde4_add_ui_files(kmobiletoolsengineui_LIB_SRCS connectionsPage.ui
   devicesfoundPage.ui
   scanprogressPage.ui
   )

#kde4_add_kcfg_files(kmobiletoolsengineui_LIB_SRCS kmobiletools_cfg.kcfgc kmobiletools_devices.kcfgc )

kde4_add_library(kmobiletoolsengineui SHARED ${kmobiletoolsengineui_LIB_SRCS})

target_link_libraries(kmobiletoolsengineui  ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} )

set_target_properties(kmobiletoolsengineui PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kmobiletoolsengineui DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

#install( FILES  kmobiletoolsengine.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES
   connectionsPage.h
   devicesfoundPage.h
   scanprogressPage.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kmobiletools)

