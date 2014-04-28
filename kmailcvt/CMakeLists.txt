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
kde_enable_exceptions()

qt5_wrap_ui(kmailcvt_SRCS ui/kselfilterpagedlg.ui ui/kimportpagedlg.ui)

kde4_add_app_icon(kmailcvt_SRCS "icons/hi*-app-kmailcvt.png")
add_executable(kmailcvt ${kmailcvt_SRCS})

target_link_libraries(kmailcvt
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::PimUtils
  mailimporter
  mailcommon
  kdepim
)

install(TARGETS kmailcvt  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

add_subdirectory(icons)
