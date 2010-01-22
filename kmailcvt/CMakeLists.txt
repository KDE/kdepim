project(kmailcvt)

  add_subdirectory( pics )

  ########### next target ###############

  set(kmailcvt_SRCS
    kimportpage.cpp
    kselfilterpage.cpp
    filters.cxx
    filter_oe.cxx
    kmailcvt.cpp
    main.cpp
    filter_pmail.cxx
    filter_plain.cxx
    filter_outlook.cxx
    filter_mbox.cxx
    filter_evolution.cxx
    filter_mailapp.cxx
    filter_evolution_v2.cxx
    filter_opera.cxx
    filter_thunderbird.cxx
    filter_kmail_maildir.cxx
    filter_sylpheed.cxx
    filter_thebat.cxx
    filter_lnotes.cxx
    filter_kmail_archive.cxx )

  include_directories(${Boost_INCLUDE_DIR} )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

  kde4_add_ui_files(kmailcvt_SRCS kselfilterpagedlg.ui kimportpagedlg.ui)

  kde4_add_app_icon(kmailcvt_SRCS "ox*-app-kmailcvt.png")
  kde4_add_executable(kmailcvt ${kmailcvt_SRCS})

  target_link_libraries(kmailcvt
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KPIMUTILS_LIBS}
    messagecore
  )

  install(TARGETS kmailcvt  ${INSTALL_TARGETS_DEFAULT_ARGS} )

  ########### install files ###############

  kde4_install_icons( ${ICON_INSTALL_DIR} )
