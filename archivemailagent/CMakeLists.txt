project(archivemailagent)

set(archivemailagent_SRCS
   archivemailkernel.cpp
   archivemailagent.cpp
)

#include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/mailimporter )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_executable(akonadi_archivemail_agent ${archivemailagent_SRCS})

target_link_libraries(akonadi_archivemail_agent
  mailcommon
)

install(TARGETS akonadi_archivemail_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES archivemailagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_archivemail_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_archivemail_agent" )


