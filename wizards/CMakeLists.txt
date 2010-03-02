project(wizards)

include_directories(${CMAKE_SOURCE_DIR}/knotes ${CMAKE_SOURCE_DIR}/kresources/lib)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


if(KDEPIM_BUILD_EVERYTHING)

  ########### next target ###############

  set(groupwarewizard_SRCS ${libknotesresources_SRCS} groupwarewizard.cpp groupwaremain.cpp overviewpage.cpp
      groupwisewizard.cpp
      sloxwizard.cpp
      kmailchanges.cpp
  )

  kde4_add_kcfg_files(groupwarewizard_SRCS groupwiseconfig.kcfgc kolabconfig.kcfgc sloxconfig.kcfgc)

  # todo: more appropriate icon?
  kde4_add_app_icon(groupwarewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

  kde4_add_executable(groupwarewizard ${groupwarewizard_SRCS})

  target_link_libraries(groupwarewizard ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS}
  kabcgroupwise kcalgroupwise
  kcal_slox kabc_slox
  kresources
  kabc kcal kdepim ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  )

  install(TARGETS groupwarewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})


  ########### next target ###############

  set(groupwisewizard_SRCS groupwisewizard.cpp groupwisemain.cpp kmailchanges.cpp)

  kde4_add_kcfg_files(groupwisewizard_SRCS groupwiseconfig.kcfgc)

  # todo: more appropriate icon?
  kde4_add_app_icon(groupwisewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

  kde4_add_executable(groupwisewizard ${groupwisewizard_SRCS})

  target_link_libraries(groupwisewizard ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KIDENTITIES_LIBS} kabcgroupwise kcalgroupwise ${KDEPIMLIBS_KCAL_LIBS} kdepim )

  install(TARGETS groupwisewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})


  ########### next target ###############

  set(sloxwizard_SRCS ${libknotesresources_SRCS} sloxwizard.cpp sloxmain.cpp)

  kde4_add_kcfg_files(sloxwizard_SRCS sloxconfig.kcfgc)

  # todo: more appropriate icon?
  kde4_add_app_icon(sloxwizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

  kde4_add_executable(sloxwizard ${sloxwizard_SRCS})

  target_link_libraries(sloxwizard ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} kcal_slox kabc_slox ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KABC_LIBS} kdepim )

  install(TARGETS sloxwizard  ${INSTALL_TARGETS_DEFAULT_ARGS})

endif(KDEPIM_BUILD_EVERYTHING)


########### next target ###############

#install( FILES kolab.kcfg DESTINATION ${KCFG_INSTALL_DIR})

if(KDEPIM_BUILD_EVERYTHING)
  install(FILES groupwarewizard.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES slox.kcfg groupwise.kcfg DESTINATION ${KCFG_INSTALL_DIR})
endif(KDEPIM_BUILD_EVERYTHING)
