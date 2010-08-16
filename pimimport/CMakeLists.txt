project(pimimport)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set( pimimport_SRCS
  main.cpp
  dumprunner.cpp
  collectiondumpjob.cpp
  resourcedumpjob.cpp
  collectionrestorejob.cpp
  resourcerestorejob.cpp
  akonadidumpassistant.cpp
)

kde4_add_executable( pimimport ${pimimport_SRCS})

target_link_libraries( pimimport
	${KDEPIMLIBS_AKONADI_LIBS}
	${KDE4_KDECORE_LIBS}
	${QT_QTCORE_LIBRARY}
)


install(TARGETS pimimport ${INSTALL_TARGETS_DEFAULT_ARGS})

