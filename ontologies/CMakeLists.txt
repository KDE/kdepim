include(NepomukAddOntologyClasses)
include_directories(${SOPRANO_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

if(SHAREDDESKTOPONTOLOGIES_VERSION VERSION_LESS 0.2)
  NEPOMUK_ADD_ONTOLOGY_CLASSES(
    nie_apps_SRCS
    FAST
    ONTOLOGIES
    ${CMAKE_CURRENT_SOURCE_DIR}/nie.rdfs
    ${CMAKE_CURRENT_SOURCE_DIR}/nco.rdfs
    ${CMAKE_CURRENT_SOURCE_DIR}/nmo.rdfs
    ${CMAKE_CURRENT_SOURCE_DIR}/ncal.rdfs
    )

  soprano_add_ontology(nie_apps_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/nie.rdfs "NIE" "Vocabulary" "rdfxml")
  soprano_add_ontology(nie_apps_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/nco.rdfs "NCO" "Vocabulary" "rdfxml")
  soprano_add_ontology(nie_apps_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/nmo.rdfs "NMO" "Vocabulary" "rdfxml")
else(SHAREDDESKTOPONTOLOGIES_VERSION VERSION_LESS 0.2)
  NEPOMUK_ADD_ONTOLOGY_CLASSES(
    nie_apps_SRCS
    FAST
    ONTOLOGIES
    ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
    ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig
    ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig
    ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/ncal.trig
    )

  soprano_add_ontology(nie_apps_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig "NIE" "Vocabulary" "trig")
  soprano_add_ontology(nie_apps_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig "NCO" "Vocabulary" "trig")
  soprano_add_ontology(nie_apps_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig "NMO" "Vocabulary" "trig")
endif(SHAREDDESKTOPONTOLOGIES_VERSION VERSION_LESS 0.2)

kde4_add_library(niefast_apps STATIC ${nie_apps_SRCS})
target_link_libraries(niefast_apps ${QT_QTCORE_LIBRARY} ${NEPOMUK_LIBRARIES})
