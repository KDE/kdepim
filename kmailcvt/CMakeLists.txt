project(kmailcvt)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_subdirectory( pics )

########### next target ###############

set(kmailcvt_SRCS
  kmailcvtkernel.cpp
  kmailcvtfilterinfogui.cpp
  kimportpage.cpp
  kselfilterpage.cpp
  kmailcvt.cpp
  main.cpp
)

include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/mailimporter ${CMAKE_SOURCE_DIR}/mailcommon ${CMAKE_SOURCE_DIR}/messagecomposer)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(kmailcvt_SRCS ui/kselfilterpagedlg.ui ui/kimportpagedlg.ui)

kde4_add_app_icon(kmailcvt_SRCS "icons/hi*-app-kmailcvt.png")
kde4_add_executable(kmailcvt ${kmailcvt_SRCS})

target_link_libraries(kmailcvt
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  mailimporter
  mailcommon
  kdepim
)

install(TARGETS kmailcvt  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

add_subdirectory(icons)
