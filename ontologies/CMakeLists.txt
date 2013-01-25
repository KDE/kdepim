include(NepomukAddOntologyClasses)
include_directories(${SOPRANO_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

soprano_add_ontology(nie_apps_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig "NIE" "Vocabulary" "trig")
soprano_add_ontology(nie_apps_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig "NCO" "Vocabulary" "trig")
soprano_add_ontology(nie_apps_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig "NMO" "Vocabulary" "trig")

kde4_add_library(niefast_apps STATIC ${nie_apps_SRCS})
target_link_libraries(niefast_apps ${QT_QTCORE_LIBRARY} ${SOPRANO_LIBRARIES} )
