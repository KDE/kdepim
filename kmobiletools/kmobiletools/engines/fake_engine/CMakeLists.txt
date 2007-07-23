
########### next target ###############

set(kmobiletools_fake_LIB_SRCS 
   fakeengine.cpp
)

kde4_add_library(kmobiletools_fake SHARED ${kmobiletools_fake_LIB_SRCS})

target_link_libraries(kmobiletools_fake kmobiletools ${KDE4_KDECORE_LIBS} )

set_target_properties(kmobiletools_fake PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletools_fake DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install( FILES  fake_engine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

