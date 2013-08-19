include_directories(${CMAKE_SOURCE_DIR}/kleopatra ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

if(KDEPIM_ONLY_KLEO)
  add_definitions(-DKDEPIM_ONLY_KLEO)
  set(_kcm_kleopatra_kleo_only_SRCS ../utils/kleo_kicondialog.cpp)
else()
  set(_kcm_kleopatra_extra_libs ${KDE4_KIO_LIBS})
endif()

if(BUILD_libkleopatraclient)

  set(_kcm_kleopatra_libkleopatraclient_extra_SRCS
    smimevalidationconfigurationwidget.cpp
    smimevalidationconfigurationpage.cpp
    cryptooperationsconfigwidget.cpp
    cryptooperationsconfigpage.cpp
  )

  kde4_add_ui_files(_kcm_kleopatra_libkleopatraclient_extra_SRCS
    smimevalidationconfigurationwidget.ui
    cryptooperationsconfigwidget.ui
  )

  kde4_add_kcfg_files(_kcm_kleopatra_libkleopatraclient_extra_SRCS
    ${CMAKE_SOURCE_DIR}/kleopatra/kcfg/smimevalidationpreferences.kcfgc
  )

  set(_kcm_kleopatra_libkleopatraclient_extra_LIBS kleopatraclientgui)

  set(_kcm_kleopatra_libkleopatraclient_extra_install_FILES
    kleopatra_config_smimevalidation.desktop
    kleopatra_config_cryptooperations.desktop
  )

else()

  set(_kcm_kleopatra_libkleopatraclient_extra_SRCS)
  set(_kcm_kleopatra_libkleopatraclient_extra_LIBS)
  set(_kcm_kleopatra_libkleopatraclient_extra_install_FILES)

endif()

set(kcm_kleopatra_PART_SRCS
  ${_kcm_kleopatra_kleo_only_SRCS}
  dirservconfigpage.cpp
  appearanceconfigpage.cpp
  appearanceconfigwidget.cpp
  gnupgsystemconfigurationpage.cpp
  ${_kcm_kleopatra_libkleopatraclient_extra_SRCS}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5152)

kde4_add_ui_files(kcm_kleopatra_PART_SRCS
  appearanceconfigwidget.ui
  smimevalidationconfigurationwidget.ui
)

kde4_add_kcfg_files(kcm_kleopatra_PART_SRCS
  ${CMAKE_SOURCE_DIR}/kleopatra/kcfg/tooltippreferences.kcfgc
  ${CMAKE_SOURCE_DIR}/kleopatra/kcfg/emailoperationspreferences.kcfgc
  ${CMAKE_SOURCE_DIR}/kleopatra/kcfg/fileoperationspreferences.kcfgc
)

if(KLEO_STATIC_KCMODULES)
  kde4_add_plugin(kcm_kleopatra STATIC ${kcm_kleopatra_PART_SRCS})
else()
  kde4_add_plugin(kcm_kleopatra ${kcm_kleopatra_PART_SRCS})
endif()

target_link_libraries(kcm_kleopatra
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  kleo
  ${_kcm_kleopatra_extra_libs}
  ${_kcm_kleopatra_libkleopatraclient_extra_LIBS}
)

if(NOT KLEO_STATIC_KCMODULES)
  install(TARGETS kcm_kleopatra DESTINATION ${PLUGIN_INSTALL_DIR})
endif()

########### install files ###############

install(FILES
  kleopatra_config_dirserv.desktop
  kleopatra_config_appear.desktop
  kleopatra_config_gnupgsystem.desktop
  ${_kcm_kleopatra_libkleopatraclient_extra_install_FILES}
  DESTINATION ${SERVICES_INSTALL_DIR}
)

