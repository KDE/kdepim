include_directories(${CMAKE_SOURCE_DIR}/akonadi/)

set(mobileui_SRCS
    listproxy.cpp
    breadcrumbnavigation.cpp
    kdeclarativemainview_p.cpp
    kdeclarativemainview.cpp
)

kde4_add_library(mobileui SHARED ${mobileui_SRCS})

target_link_libraries(mobileui
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_LIBRARY}
  akonadi_next
)

install(TARGETS mobileui ${INSTALL_TARGETS_DEFAULT_ARGS})
