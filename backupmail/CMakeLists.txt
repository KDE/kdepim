project(backupmail)

########### next target ###############

set(backupmail_SRCS
  main.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_executable(backupmail_SRCS ${backupmail_SRCS})

target_link_libraries(backupmail
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  mailimporter
  mailcommon
  kdepim
)

install(TARGETS backupmail ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )
