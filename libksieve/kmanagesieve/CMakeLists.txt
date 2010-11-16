set(kmanagesieve_LIB_SRCS
  response.cpp
  session.cpp
  sievejob.cpp
)

kde4_add_library(kmanagesieve ${LIBRARY_TYPE} ${kmanagesieve_LIB_SRCS})

target_link_libraries(kmanagesieve
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
)

set_target_properties(kmanagesieve PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kmanagesieve ${INSTALL_TARGETS_DEFAULT_ARGS})
