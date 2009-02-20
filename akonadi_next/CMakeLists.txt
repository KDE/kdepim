project(akonadi_next)

find_package(KDE4 REQUIRED)
find_package(Akonadi REQUIRED)
find_package(KdepimLibs REQUIRED)

include_directories(
  ${AKONADI_INCLUDE_DIR}
  ${KDE4_INCLUDES}
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${QT_INCLUDES}
)


set( akonadi_next_SRCS
  entitytreemodel.cpp
  entitytreemodel_p.cpp
  entityupdateadapter.cpp
  descendantentitiesproxymodel.cpp
  entityfilterproxymodel.cpp
  entitytreeview.cpp
  collectionchildorderattribute.cpp
  modeltest.cpp
)

kde4_add_library(akonadi_next ${akonadi_next_SRCS} )

target_link_libraries(akonadi_next
                      ${AKONADI_COMMON_LIBRARIES}
)
