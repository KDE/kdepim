project(wizards)

include_directories(${CMAKE_SOURCE_DIR}/kcal ${CMAKE_SOURCE_DIR}/kresources/kolab/shared ${CMAKE_SOURCE_DIR}/knotes ${CMAKE_SOURCE_DIR}/kresources/lib)

message( STATUS "GroupWise wizard disabled." )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(groupwarewizard_SRCS ${libknotesresources_SRCS} groupwarewizard.cpp groupwaremain.cpp overviewpage.cpp 
    egroupwarewizard.cpp
    sloxwizard.cpp
    kolabwizard.cpp kmailchanges.cpp kolabkmailchanges.cpp 
)

kde4_add_kcfg_files(groupwarewizard_SRCS egroupwareconfig.kcfgc kolabconfig.kcfgc sloxconfig.kcfgc)

kde4_add_executable(groupwarewizard ${groupwarewizard_SRCS})

target_link_libraries(groupwarewizard  ${KDE4_KDECORE_LIBS} 
kabc_xmlrpc kcal_xmlrpc knotes_xmlrpc
kcal_slox kabc_slox
kabckolab knoteskolab
kcalkolab kresources
kabc kcal kdepim ${KDE4_KPIMIDENTITIES_LIBS}
)

install(TARGETS groupwarewizard  DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

set(egroupwarewizard_SRCS ${libknotesresources_SRCS} egroupwarewizard.cpp egroupwaremain.cpp)


kde4_add_kcfg_files(egroupwarewizard_SRCS egroupwareconfig.kcfgc)

kde4_add_executable(egroupwarewizard ${egroupwarewizard_SRCS})

target_link_libraries(egroupwarewizard  ${KDE4_KDECORE_LIBS} kabc_xmlrpc kcal_xmlrpc knotes_xmlrpc ${KDE4_KCAL_LIBS} kdepim)

install(TARGETS egroupwarewizard  DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

set(sloxwizard_SRCS ${libknotesresources_SRCS} sloxwizard.cpp sloxmain.cpp)


kde4_add_kcfg_files(sloxwizard_SRCS sloxconfig.kcfgc)

kde4_add_executable(sloxwizard ${sloxwizard_SRCS})

target_link_libraries(sloxwizard  ${KDE4_KDECORE_LIBS} kcal_slox kabc_slox ${KDE4_KCAL_LIBS} ${KDE4_KABC_LIBS} kdepim)

install(TARGETS sloxwizard  DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############

set(kolabwizard_SRCS ${libknotesresources_SRCS} 
   kolabwizard.cpp 
   kmailchanges.cpp 
   kolabkmailchanges.cpp 
   kolabmain.cpp)


kde4_add_kcfg_files(kolabwizard_SRCS kolabconfig.kcfgc)

kde4_add_executable(kolabwizard ${kolabwizard_SRCS})

target_link_libraries(kolabwizard  ${KDE4_KDECORE_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KPIMIDENTITIES_LIBS} kabckolab knoteskolab kcalkolab kdepim)

install(TARGETS kolabwizard  DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install( FILES groupwarewizard.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES egroupware.kcfg slox.kcfg kolab.kcfg groupwise.kcfg  DESTINATION ${KCFG_INSTALL_DIR})

