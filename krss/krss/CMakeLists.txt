
add_subdirectory(tests)

# Nepomuk is not ready for being the default tag provider
# so better disable it
SET( Nepomuk_FOUND FALSE )
macro_bool_to_01(Nepomuk_FOUND HAVE_NEPOMUK)
configure_file(config-nepomuk.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-nepomuk.h)

if (Nepomuk_FOUND)
    # Yes, Soprano includes is what we need here
    include_directories( ${SOPRANO_INCLUDE_DIR} ${NEPOMUK_INCLUDE_DIR} )
endif (Nepomuk_FOUND)

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
   rssitem.h  #TODO review if rssitem.h should be private
)

kde4_add_library(krss ${LIBRARY_TYPE} ${krss_LIB_SRCS})

if(KRSS_ENABLE_PROTOBUF_SERIALIZER)
endif(KRSS_ENABLE_PROTOBUF_SERIALIZER)

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
