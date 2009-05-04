project(wizards)

include_directories(${CMAKE_SOURCE_DIR}/kcal ${CMAKE_SOURCE_DIR}/kresources/kolab/shared ${CMAKE_SOURCE_DIR}/knotes ${CMAKE_SOURCE_DIR}/kresources/lib)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

if(BUILD_EVERYTHING)
set(groupwarewizard_SRCS ${libknotesresources_SRCS} groupwarewizard.cpp groupwaremain.cpp overviewpage.cpp 
    egroupwarewizard.cpp
    groupwisewizard.cpp
    sloxwizard.cpp
    kolabwizard.cpp kmailchanges.cpp kolabkmailchanges.cpp
)

kde4_add_kcfg_files(groupwarewizard_SRCS egroupwareconfig.kcfgc groupwiseconfig.kcfgc kolabconfig.kcfgc sloxconfig.kcfgc)


# todo: more appropriate icon?
kde4_add_app_icon(groupwarewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

kde4_add_executable(groupwarewizard ${groupwarewizard_SRCS})

target_link_libraries(groupwarewizard  ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} 
kabc_xmlrpc kcal_xmlrpc knotes_xmlrpc
kabcgroupwise kcalgroupwise
kcal_slox kabc_slox
kabckolab knoteskolab
kcalkolab kresources
kabc kcal kdepim ${KDE4_KPIMIDENTITIES_LIBS}
)

install(TARGETS groupwarewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_EVERYTHING)

########### next target ###############

if(BUILD_EVERYTHING)
set(egroupwarewizard_SRCS ${libknotesresources_SRCS} egroupwarewizard.cpp egroupwaremain.cpp)


kde4_add_kcfg_files(egroupwarewizard_SRCS egroupwareconfig.kcfgc)

# todo: more appropriate icon?
kde4_add_app_icon(egroupwarewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

kde4_add_executable(egroupwarewizard ${egroupwarewizard_SRCS})

target_link_libraries(egroupwarewizard  ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} kabc_xmlrpc kcal_xmlrpc knotes_xmlrpc ${KDE4_KCAL_LIBS} kdepim)

install(TARGETS egroupwarewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_EVERYTHING)

########### next target ###############

if(BUILD_EVERYTHING)
set(groupwisewizard_SRCS groupwisewizard.cpp groupwisemain.cpp kmailchanges.cpp)

kde4_add_kcfg_files(groupwisewizard_SRCS groupwiseconfig.kcfgc)

# todo: more appropriate icon?
kde4_add_app_icon(groupwisewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

kde4_add_executable(groupwisewizard ${groupwisewizard_SRCS})

target_link_libraries(groupwisewizard ${QT_QT3SUPPORT_LIBRARY}${KDE4_KDECORE_LIBS} kabcgroupwise kcalgroupwise ${KDE4_KCAL_LIBS} kdepim kpimidentities)

install(TARGETS groupwisewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_EVERYTHING)

########### next target ###############

if(BUILD_EVERYTHING)
set(sloxwizard_SRCS ${libknotesresources_SRCS} sloxwizard.cpp sloxmain.cpp)


kde4_add_kcfg_files(sloxwizard_SRCS sloxconfig.kcfgc)

# todo: more appropriate icon?
kde4_add_app_icon(sloxwizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

kde4_add_executable(sloxwizard ${sloxwizard_SRCS})

target_link_libraries(sloxwizard   ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} kcal_slox kabc_slox ${KDE4_KCAL_LIBS} ${KDE4_KABC_LIBS} kdepim)

install(TARGETS sloxwizard  ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_EVERYTHING)

########### next target ###############

set(kolabwizard_SRCS ${libknotesresources_SRCS} 
   kolabwizard.cpp 
   kmailchanges.cpp 
   kolabkmailchanges.cpp 
   kolabmain.cpp)


kde4_add_kcfg_files(kolabwizard_SRCS kolabconfig.kcfgc)

# todo: more appropriate icon?
kde4_add_app_icon(kolabwizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

kde4_add_executable(kolabwizard ${kolabwizard_SRCS})

target_link_libraries(kolabwizard   ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KCAL_LIBS} ${KDE4_KPIMIDENTITIES_LIBS} kabckolab knoteskolab kcalkolab kdepim)

install(TARGETS kolabwizard  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

if(BUILD_EVERYTHING)
set(scalixwizard_SRCS ${libknotesresources_SRCS} 
   scalixwizard.cpp 
   kmailchanges.cpp 
   scalixkmailchanges.cpp 
   scalixmain.cpp)


kde4_add_kcfg_files(scalixwizard_SRCS scalixconfig.kcfgc)

# todo: more appropriate icon?
kde4_add_app_icon(scalixwizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

kde4_add_executable(scalixwizard ${scalixwizard_SRCS})

target_link_libraries(scalixwizard  ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KCAL_LIBS} ${KDE4_KPIMIDENTITIES_LIBS} kabcscalix knotesscalix kcalscalix kdepim)

install(TARGETS scalixwizard  ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(BUILD_EVERYTHING)

########### install files ###############

install( FILES kolab.kcfg DESTINATION ${KCFG_INSTALL_DIR})
if(BUILD_EVERYTHING)
  install(FILES groupwarewizard.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES egroupware.kcfg slox.kcfg kolab.kcfg groupwise.kcfg scalix.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
endif(BUILD_EVERYTHING)
