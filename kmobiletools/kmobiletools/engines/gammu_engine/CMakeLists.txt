
include_directories( ${CMAKE_SOURCE_DIR}/kmobiletools/kmobiletools/libkmobiletools ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kmobiletools_gammu_LIB_SRCS 
   kmobiletoolsgammu_engine.cpp 
   gammu_jobs.cpp 
   qphoneerror.cpp 
   device.cpp )


kde4_add_library(kmobiletools_gammu SHARED ${kmobiletools_gammu_LIB_SRCS})

target_link_libraries(kmobiletools_gammu  ${KDE4_KDECORE_LIBS} Gammu kmobiletools )

set_target_properties(kmobiletools_gammu PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletools_gammu ${INSTALL_TARGETS_DEFAULT_ARGS})

