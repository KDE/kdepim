project(thunderbirdaddressbookimporter)

set(thunderbirdaddressbookimporter_SRCS
    MorkParser.cpp
)

kde4_add_executable(thunderbirdaddressbookimporter ${thunderbirdaddressbookimporter_SRCS})

target_link_libraries(thunderbirdaddressbookimporter
  ${KDE4_KIO_LIBS}
)

install(TARGETS thunderbirdaddressbookimporter  ${INSTALL_TARGETS_DEFAULT_ARGS} )
