project(kmailcvt)

add_subdirectory( pics )

########### next target ###############

set(kmailcvt_SRCS
  filterinfo.cpp
  kimportpage.cpp
  kselfilterpage.cpp
  filters.cpp
  filter_oe.cpp
  kmailcvt.cpp
  main.cpp
  filter_pmail.cpp
  filter_plain.cpp
  filter_outlook.cpp
  filter_mbox.cpp
  filter_evolution.cpp
  filter_mailapp.cpp
  filter_evolution_v2.cpp
  filter_evolution_v3.cpp
  filter_opera.cpp
  filter_thunderbird.cpp
  filter_kmail_maildir.cpp
  filter_sylpheed.cpp
  filter_thebat.cpp
  filter_lnotes.cpp
  filter_kmail_archive.cpp )

include_directories(${Boost_INCLUDE_DIR} )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(kmailcvt_SRCS kselfilterpagedlg.ui kimportpagedlg.ui)

kde4_add_app_icon(kmailcvt_SRCS "ox*-app-kmailcvt.png")
kde4_add_executable(kmailcvt ${kmailcvt_SRCS})

target_link_libraries(kmailcvt
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  messagecore
)

install(TARGETS kmailcvt  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )
