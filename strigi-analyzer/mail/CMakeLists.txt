
include_directories(
        ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(mailendanalyzer_SRCS
  mailendanalyzer.cpp
  messageanalyzer.cpp
)
kde4_add_kcfg_files(mailendanalyzer_SRCS settings.kcfgc)

include_directories(${CMAKE_SOURCE_DIR}/strigi-analyzer ${STRIGI_INCLUDE_DIR})
add_definitions(-DMAKE_PIMSTRIGIANALYZER_LIB)

kde4_add_library(mail MODULE ${mailendanalyzer_SRCS})

target_link_libraries(mail
  messagecore
  messageviewer
  ${KDE4_KFILE_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${STRIGI_STREAMS_LIBRARY}
  ${STRIGI_STREAMANALYZER_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBS}
)
set_target_properties(mail PROPERTIES PREFIX strigiea_)
install(TARGETS mail LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)
