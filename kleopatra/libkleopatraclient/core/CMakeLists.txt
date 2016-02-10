if(ASSUAN2_FOUND)
  include_directories(${ASSUAN2_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra)
else()
  include_directories(${ASSUAN_INCLUDES} ${CMAKE_SOURCE_DIR}/kleopatra)
endif()

add_definitions(
  -D_ASSUAN_ONLY_GPG_ERRORS
  -DQT_NO_CAST_FROM_ASCII -DQT_NO_KEYWORDS -DQT_NO_CAST_TO_ASCII
)

if(WIN32)
  set(_kleopatraclientcore_extra_SRCS ../../utils/gnupg-registry.c)
else()
  set(_kleopatraclientcore_extra_SRCS)
endif()

ecm_qt_declare_logging_category(_kleopatraclientcore_common_SRCS HEADER libkleopatraclientcore_debug.h IDENTIFIER LIBKLEOPATRACLIENTCORE_LOG CATEGORY_NAME log_libkleopatraclientcore)

add_library(kleopatraclientcore
  ${_kleopatraclientcore_extra_SRCS}
  initialization.cpp
  command.cpp
  selectcertificatecommand.cpp
  signencryptfilescommand.cpp
  decryptverifyfilescommand.cpp
  libkleopatraclientcore_debug.cpp
  ${_kleopatraclientcore_common_SRCS}
)
generate_export_header(kleopatraclientcore BASE_NAME kleopatraclientcore)

set_target_properties(kleopatraclientcore PROPERTIES
  VERSION ${libkleopatraclient_version}
  SOVERSION ${libkleopatraclient_soversion}
)

if(WIN32)
  if(ASSUAN2_FOUND)
    target_link_libraries(kleopatraclientcore
      
      ${ASSUAN2_LIBRARIES}
      ws2_32
    )
  else()
    target_link_libraries(kleopatraclientcore
      
      ${ASSUAN_VANILLA_LIBRARIES}
      ws2_32
    )
  endif()
else()
  if(ASSUAN2_FOUND)
    target_link_libraries(kleopatraclientcore
      
      ${ASSUAN2_LIBRARIES}
    )
  else()
    target_link_libraries(kleopatraclientcore
      
      ${ASSUAN_PTHREAD_LIBRARIES}
    )
  endif()
endif()

target_link_libraries(kleopatraclientcore Qt5::Widgets KF5::I18n)

install(TARGETS kleopatraclientcore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
