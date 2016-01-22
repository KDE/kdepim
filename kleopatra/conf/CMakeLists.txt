include_directories(${kleopatra_SOURCE_DIR})

if(KDEPIM_ONLY_KLEO)
  add_definitions(-DKDEPIM_ONLY_KLEO)
  set(_kcm_kleopatra_kleo_only_SRCS ../utils/kleo_kicondialog.cpp)
endif()

if(BUILD_libkleopatraclient)

  set(_kcm_kleopatra_libkleopatraclient_extra_SRCS
    smimevalidationconfigurationwidget.cpp
    smimevalidationconfigurationpage.cpp
    cryptooperationsconfigwidget.cpp
    cryptooperationsconfigpage.cpp
  )

  ki18n_wrap_ui(_kcm_kleopatra_libkleopatraclient_extra_SRCS
    smimevalidationconfigurationwidget.ui
    cryptooperationsconfigwidget.ui
  )

  kconfig_add_kcfg_files(_kcm_kleopatra_libkleopatraclient_extra_SRCS
    ${kleopatra_SOURCE_DIR}/kcfg/smimevalidationpreferences.kcfgc
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
  ${kleopatra_BINARY_DIR}/kleopatra_debug.cpp
  ${_kcm_kleopatra_libkleopatraclient_extra_SRCS}
)


ki18n_wrap_ui(kcm_kleopatra_PART_SRCS
  appearanceconfigwidget.ui
  smimevalidationconfigurationwidget.ui
)

kconfig_add_kcfg_files(kcm_kleopatra_PART_SRCS
  ${kleopatra_SOURCE_DIR}/kcfg/tooltippreferences.kcfgc
  ${kleopatra_SOURCE_DIR}/kcfg/emailoperationspreferences.kcfgc
  ${kleopatra_SOURCE_DIR}/kcfg/fileoperationspreferences.kcfgc
)

if(KLEO_STATIC_KCMODULES)
  add_library(kcm_kleopatra MODULE STATIC ${kcm_kleopatra_PART_SRCS})
else()
  add_library(kcm_kleopatra MODULE ${kcm_kleopatra_PART_SRCS})
endif()

target_link_libraries(kcm_kleopatra
  KF5::Libkleo
  KF5::KCMUtils
  KF5::IconThemes
  KF5::I18n
  Qt5::DBus
  ${_kcm_kleopatra_extra_libs}
  ${_kcm_kleopatra_libkleopatraclient_extra_LIBS}
)

if(NOT KLEO_STATIC_KCMODULES)
  install(TARGETS kcm_kleopatra DESTINATION ${KDE_INSTALL_PLUGINDIR})
endif()

########### install files ###############

install(FILES
  kleopatra_config_dirserv.desktop
  kleopatra_config_appear.desktop
  kleopatra_config_gnupgsystem.desktop
  ${_kcm_kleopatra_libkleopatraclient_extra_install_FILES}
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

