
if ( ${STRIGI_VERSION} VERSION_GREATER 0.7.2 )
include_directories(
        ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(mailendanalyzer_SRCS
  mailendanalyzer.cpp
  messageanalyzer.cpp
)
kde4_add_kcfg_files(mailendanalyzer_SRCS settings.kcfgc)

include_directories(${CMAKE_SOURCE_DIR}/messageviewer ${CMAKE_SOURCE_DIR}/strigi-analyzer ${STRIGI_INCLUDE_DIR})
add_definitions(-DMAKE_PIMSTRIGIANALYZER_LIB)

if(WINCE)
kde4_add_library(mail STATIC ${mailendanalyzer_SRCS})
else()
kde4_add_library(mail MODULE ${mailendanalyzer_SRCS})
endif()

target_link_libraries(mail
  messagecore
  messageviewer
  ${QT_QTCORE_LIBRARY}
  ${STRIGI_STREAMS_LIBRARY}
  ${STRIGI_STREAMANALYZER_LIBRARY}
  ${KDEPIMLIBS_KMIME_LIBS}
)
if (NOT WINCE)
target_link_libraries(mail ${KDE4_KFILE_LIBS} )
endif()
set_target_properties(mail PROPERTIES PREFIX strigiea_)
install(TARGETS mail LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi
                    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}/strigi)
endif ()
