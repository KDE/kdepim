include_directories(${CMAKE_SOURCE_DIR}/libkdepim)
########### next target ###############

set(kabcommon_LIB_SRCS 
   kabprefs.cpp
   locationmap.cpp
   filter.cpp
    )

kde4_add_kcfg_files(kabcommon_LIB_SRCS kabprefs_base.kcfgc)
kde4_add_library(kabcommon SHARED ${kabcommon_LIB_SRCS})
target_link_libraries(kabcommon kdepim ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS})
set_target_properties(kabcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kabcommon ${INSTALL_TARGETS_DEFAULT_ARGS})

