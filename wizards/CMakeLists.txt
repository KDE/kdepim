project(wizards)

include_directories(${CMAKE_SOURCE_DIR}/knotes ${CMAKE_SOURCE_DIR}/kresources/lib)

add_subdirectory(tests)

if(KDEPIM_BUILD_EVERYTHING)

  set(wizardprivate_SRCS
      kconfigpropagator.cpp
      kconfigwizard.cpp
  )

  kde4_add_library(wizardprivate STATIC ${wizardprivate_SRCS})

  ########### next target ###############

  set(groupwarewizard_SRCS ${libknotesresources_SRCS} groupwarewizard.cpp groupwaremain.cpp overviewpage.cpp
      groupwisewizard.cpp
      sloxwizard.cpp
      kmailchanges.cpp
  )

  kde4_add_kcfg_files(groupwarewizard_SRCS groupwiseconfig.kcfgc sloxconfig.kcfgc)

  # todo: more appropriate icon?
  kde4_add_app_icon(groupwarewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

  kde4_add_executable(groupwarewizard ${groupwarewizard_SRCS})

  target_link_libraries(groupwarewizard
                        ${QT_QTXML_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        ${KDE4_KDEUI_LIBS}
                        ${KDE4_KIO_LIBS}
                        ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
                        ${KDEPIMLIBS_KPIMUTILS_LIBS}
                        kabc
                        kabcgroupwise
                        kabc_slox
                        kcal
                        kcalgroupwise
                        kcal_slox
                        kresources
                        wizardprivate)

  install(TARGETS groupwarewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})


  ########### next target ###############

  set(groupwisewizard_SRCS groupwisewizard.cpp groupwisemain.cpp kmailchanges.cpp)

  kde4_add_kcfg_files(groupwisewizard_SRCS groupwiseconfig.kcfgc)

  # todo: more appropriate icon?
  kde4_add_app_icon(groupwisewizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

  kde4_add_executable(groupwisewizard ${groupwisewizard_SRCS})

  target_link_libraries(groupwisewizard
                        ${QT_QTXML_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        ${KDE4_KIDENTITIES_LIBS}
                        ${KDE4_KIO_LIBS}
                        ${KDEPIMLIBS_KCAL_LIBS}
                        ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
                        ${KDEPIMLIBS_KPIMUTILS_LIBS}
                        kabcgroupwise
                        kcalgroupwise
                        wizardprivate)

  install(TARGETS groupwisewizard  ${INSTALL_TARGETS_DEFAULT_ARGS})


  ########### next target ###############

  set(sloxwizard_SRCS ${libknotesresources_SRCS} sloxwizard.cpp sloxmain.cpp)

  kde4_add_kcfg_files(sloxwizard_SRCS sloxconfig.kcfgc)

  # todo: more appropriate icon?
  kde4_add_app_icon(sloxwizard_SRCS "${KDE4_ICON_DIR}/oxygen/*/actions/tools-wizard.png")

  kde4_add_executable(sloxwizard ${sloxwizard_SRCS})

  target_link_libraries(sloxwizard
                        ${QT_QTXML_LIBRARY}
                        ${KDE4_KDECORE_LIBS}
                        ${KDE4_KIO_LIBS}
                        ${KDEPIMLIBS_KABC_LIBS}
                        ${KDEPIMLIBS_KCAL_LIBS}
                        kabc_slox
                        kcal_slox
                        wizardprivate)

  install(TARGETS sloxwizard  ${INSTALL_TARGETS_DEFAULT_ARGS})

endif(KDEPIM_BUILD_EVERYTHING)


########### next target ###############

if(KDEPIM_BUILD_EVERYTHING)
  install(FILES groupwarewizard.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES slox.kcfg groupwise.kcfg DESTINATION ${KCFG_INSTALL_DIR})
endif(KDEPIM_BUILD_EVERYTHING)
