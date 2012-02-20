
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
# ui stuff
   ui/feedlistview.cpp
   ui/feedpropertiesdialog.cpp
)

if(KRSS_ENABLE_PROTOBUF_SERIALIZER)
    WRAP_PROTO(krss_LIB_PROTOBUF_SRCS krss.proto)

    set(krss_LIB_SRCS
        ${krss_LIB_SRCS}
        ${krss_LIB_PROTOBUF_SRCS}
        rssitemserializer_protobuf.cpp
    )
    set(KRSS_EXTRA_LIBS ${PROTOBUF_LIBRARY})
else(KRSS_ENABLE_PROTOBUF_SERIALIZER)
if(KRSS_ENABLE_QDATASTREAM_SERIALIZER)
    set(krss_LIB_SRCS
        ${krss_LIB_SRCS}
        rssitemserializer_qdatastream.cpp
    )
    set(KRSS_EXTRA_LIBS ${PROTOBUF_LIBRARY})
else(KRSS_ENABLE_QDATASTREAM_SERIALIZER)
if(KRSS_ENABLE_JSON_SERIALIZER)
   set(krss_LIB_SRCS
       ${krss_LIB_SRCS}
       rssitemserializer_json.cpp
   )
   set(KRSS_EXTRA_LIBS qjson)
else(KRSS_ENABLE_JSON_SERIALIZER)
   set(krss_LIB_SRCS
       ${krss_LIB_SRCS}
       rssitemserializer_xml.cpp
   )
endif(KRSS_ENABLE_JSON_SERIALIZER)
endif(KRSS_ENABLE_QDATASTREAM_SERIALIZER)
endif(KRSS_ENABLE_PROTOBUF_SERIALIZER)

set(krss_LIB_HDRS
   krss_export.h
   item.h
   enclosure.h
   person.h
   category.h
   feeditemmodel.h
   rssitem.h  #TODO review if rssitem.h should be private
)

set(krss_UI_LIB_HDRS
   ui/feedlistview.h
   ui/netfeedcreatedialog.h
   ui/feedpropertiesdialog.h
)

set(krss_PRIVATE_LIB_HDRS
    rssitemserializer.h
)

kde4_add_ui_files(krss_LIB_SRCS ui/netfeedcreatedialog.ui ui/feedpropertiesdialog.ui)

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

if (Nepomuk_FOUND)
    target_link_libraries(krss ${NEPOMUK_LIBRARIES} ${SOPRANO_LIBRARIES})
endif (Nepomuk_FOUND)

set_target_properties(krss PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krss ${INSTALL_TARGETS_DEFAULT_ARGS})
