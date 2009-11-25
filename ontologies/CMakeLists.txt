include(NepomukAddOntologyClasses)
NEPOMUK_ADD_ONTOLOGY_CLASSES(
  nie_apps_SRCS
  FAST
  ONTOLOGIES
  ${CMAKE_CURRENT_SOURCE_DIR}/nie.rdfs
  ${CMAKE_CURRENT_SOURCE_DIR}/nfo.rdfs
  ${CMAKE_CURRENT_SOURCE_DIR}/nco.rdfs
  ${CMAKE_CURRENT_SOURCE_DIR}/nmo.rdfs
  ${CMAKE_CURRENT_SOURCE_DIR}/ncal.rdfs
  )

include_directories(${SOPRANO_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

soprano_add_ontology(nie_apps_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/nie.rdfs "NIE" "Vocabulary" "rdfxml")
soprano_add_ontology(nie_apps_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/nco.rdfs "NCO" "Vocabulary" "rdfxml")
soprano_add_ontology(nie_apps_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/nmo.rdfs "NMO" "Vocabulary" "rdfxml")

kde4_add_library(niefast_apps STATIC ${nie_apps_SRCS})
target_link_libraries(niefast_apps ${QT_QTCORE_LIBRARY} ${NEPOMUK_LIBRARIES})
