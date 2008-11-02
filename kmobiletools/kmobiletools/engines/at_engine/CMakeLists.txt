
########### next target ###############

set(kmobiletools_at_LIB_SRCS 
   at_engine.cpp 
   at_jobs.cpp 
   atabilities.cpp 
   phonebook_jobs.cpp 
   testphonedevice.cpp 
   sms_jobs.cpp 
   #calendar_jobs.cpp 
   wizard/at_connectionspage.cpp
   cfgwidgets/engineconfig.cpp
   wizard/bluetooth.cpp
   wizard/serial.cpp
   wizard/userconnection.cpp
   wizard/at_scanprogresspage.cpp
   wizard/at_devicesfoundpage.cpp
)

kde4_add_kcfg_files(kmobiletools_at_LIB_SRCS atengineconfig.kcfgc )
kde4_add_ui_files(kmobiletools_at_LIB_SRCS cfgwidgets/engineconfig.ui
wizard/bluetooth.ui wizard/serial.ui wizard/userconnection.ui )
kde4_add_library(kmobiletools_at SHARED ${kmobiletools_at_LIB_SRCS})

target_link_libraries(kmobiletools_at  ${KDE4_KDECORE_LIBS} kmobiletools kmobiletoolsengineui)

set_target_properties(kmobiletools_at PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletools_at ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES atengineconfig.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})

########### install files ###############

install(FILES  at_engine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

