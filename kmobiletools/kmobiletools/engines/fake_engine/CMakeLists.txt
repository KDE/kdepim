
########### next target ###############

set(kmobiletools_fake_LIB_SRCS 
   fakeengine.cpp
   fakeengine.h
   jobs/fetchaddressbookjob.cpp
   jobs/fetchaddressbookjob.h
)

kde4_add_plugin(kmobiletools_fake ${kmobiletools_fake_LIB_SRCS})
target_link_libraries(kmobiletools_fake kmobiletoolslib ${KDE4_KDECORE_LIBS} ${KDE4_KABC_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${QT_QTGUI_LIBRARIES})
install(TARGETS kmobiletools_fake DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES  fake_engine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

