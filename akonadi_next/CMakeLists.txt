project(akonadi_next)

set( akonadi_next_SRCS
  entitytreemodel.cpp
  entitytreemodel_p.cpp
  entityupdateadapter.cpp
  clientsideentitystorage.cpp
  clientsideentitystorage_p.cpp
  descendantentitiesmodel.cpp
  descendantentitiesmodel_p.cpp
  entityfilterproxymodel.cpp
  entitytreeview.cpp
  collectionchildorderattribute.cpp
  modeltest.cpp
)

kde4_add_library(akonadi_next ${akonadi_next_SRCS} )

target_link_libraries(akonadi_next
                      ${AKONADI_COMMON_LIBRARIES}
)
