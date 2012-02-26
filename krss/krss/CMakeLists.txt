add_subdirectory(tests)

set(krss_LIB_SRCS
   item.cpp
   enclosure.cpp
   person.cpp
   category.cpp
   rssitem.cpp
   feedpropertiescollectionattribute.cpp
   feedcollection.cpp
   helper.cpp
   feeditemmodel.cpp
)

set(krss_LIB_HDRS
   krss_export.h
   item.h
   enclosure.h
   person.h
   category.h
   feeditemmodel.h
   rssitem.h
)


kde4_add_library(krss ${LIBRARY_TYPE} ${krss_LIB_SRCS})

target_link_libraries(krss
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_AKONADI_LIBS}
                      ${KDEPIMLIBS_SYNDICATION_LIBS}
                      ${KRSS_EXTRA_LIBS}
)

set_target_properties(krss PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krss ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES
   krss_export.h
   item.h
   enclosure.h
   person.h
   category.h
   feeditemmodel.h
   rssitem.h  #TODO review if rssitem.h should be private
   DESTINATION ${INCLUDE_INSTALL_DIR}/krss COMPONENT Devel)
