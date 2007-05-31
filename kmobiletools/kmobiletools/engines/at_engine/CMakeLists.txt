
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
)

kde4_automoc(${kmobiletools_at_LIB_SRCS})
kde4_add_kcfg_files(kmobiletools_at_LIB_SRCS atengineconfig.kcfgc )
kde4_add_ui_files(kmobiletools_at_LIB_SRCS cfgwidgets/engineconfig.ui
wizard/bluetooth.ui wizard/serial.ui wizard/userconnection.ui )
kde4_add_library(kmobiletools_at SHARED ${kmobiletools_at_LIB_SRCS})

target_link_libraries(kmobiletools_at  ${KDE4_KDECORE_LIBS} kmobiletools kmobiletoolsengineui )

set_target_properties(kmobiletools_at PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kmobiletools_at DESTINATION ${LIB_INSTALL_DIR} )
install( FILES atengineconfig.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )

########### install files ###############

install( FILES  at_engine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

