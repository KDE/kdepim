

set(kmobiletoolsengineui_LIB_SRCS 
   connectionsPage.h
   connectionsPage.cpp
   scanprogressPage.h
   scanprogressPage.cpp
   devicesfoundPage.h
   devicesfoundPage.cpp
   )


kde4_add_ui_files(kmobiletoolsengineui_LIB_SRCS connectionsPage.ui
   devicesfoundPage.ui
   scanprogressPage.ui
   )

#kde4_add_kcfg_files(kmobiletoolsengineui_LIB_SRCS kmobiletools_cfg.kcfgc kmobiletools_devices.kcfgc )

kde4_add_library(kmobiletoolsengineui SHARED ${kmobiletoolsengineui_LIB_SRCS})

target_link_libraries(kmobiletoolsengineui  ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} )

set_target_properties(kmobiletoolsengineui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletoolsengineui ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

#install( FILES  kmobiletoolsengine.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES
   connectionsPage.h
   devicesfoundPage.h
   scanprogressPage.h
   engineui_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/libkmobiletoolsengineui)

