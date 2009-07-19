# needed to compile the code in subdirectories
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tests)

# taken from dolphin
macro_optional_find_package(Nepomuk)
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
   tag.cpp
   feedpropertiescollectionattribute.cpp
   subscriptionlabelscollectionattribute.cpp
   tagidsattribute.cpp
   feedcollection.cpp
   resourcemanager.cpp
   resource.cpp
   netresource.cpp
   #searchresource.cpp
   retrieveresourcecollectionsjob.cpp
   feed.cpp
   feedvisitor.cpp
   searchfeed.cpp
   netfeed.cpp
   feedlist.cpp
   dbushelper.cpp
   itemmodel.cpp
   treenode.cpp
   treenodevisitor.cpp
   feedlistmodel.cpp
   exportopmljob.cpp
   importitemsjob.cpp
   importopmljob.cpp
   itemlisting.cpp
   itemlistjob.cpp
   feedjobs.cpp
   itemjobs.cpp
   statusmodifyjob.cpp
   netfeedcreatejob.cpp
   standardactionmanager.cpp
   #subscriptionsmodel.cpp
   tagjobs.cpp
   tagprovider.cpp
# ui stuff
   ui/itemview.cpp
   ui/feedlistview.cpp
   ui/feedlistdelegate.cpp
   ui/netfeedcreatedialog.cpp
   ui/feedpropertiesdialog.cpp
   ui/tagpropertiesdialog.cpp
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

if(Nepomuk_FOUND)
set(krss_LIB_SRCS
   ${krss_LIB_SRCS}
   nepomuktagprovider/nepomuktagprovider.cpp
   nepomuktagprovider/nepomuktagjobimpls.cpp
)
else(Nepomuk_FOUND)
set(krss_LIB_SRCS
   ${krss_LIB_SRCS}
   defaulttagprovider/tagpropertiesattribute.cpp
   defaulttagprovider/defaulttagprovider.cpp
   defaulttagprovider/defaulttagjobimpls.cpp
)
endif(Nepomuk_FOUND)

set(krss_LIB_HDRS
   krss_export.h
   item.h
   enclosure.h
   exportopmljob.h
   person.h
   category.h
   tag.h
   resourcemanager.h
   resource.h
   feed.h
   feedvisitor.h
   searchfeed.h
   netfeed.h
   treenode.h
   treenodevisitor.h
   feedlist.h
   itemlisting.h
   itemlistjob.h
   itemmodel.h
   importitemsjob.h
   importopmljob.h
   feedlistmodel.h
   itemjobs.h
   feedjobs.h
   statusmodifyjob.h
   netfeedcreatejob.h
   standardactionmanager.h
   subscriptionsmodel.h
   tagjobs.h
   tagprovider.h
)

set(krss_UI_LIB_HDRS
   ui/itemview.h
   ui/feedlistview.h
   ui/netfeedcreatedialog.h
   ui/feedpropertiesdialog.h
   ui/tagpropertiesdialog.h
)

set(krss_PRIVATE_LIB_HDRS
    rssitem.h
    rssitemserializer.h
)

# WARNING: we depend on the dbus interface description from krss/krssresource
qt4_add_dbus_interface(krss_LIB_SRCS ${CMAKE_SOURCE_DIR}/krss/krssresource/org.kde.krss.xml krssinterface)
kde4_add_ui_files(krss_LIB_SRCS ui/netfeedcreatedialog.ui ui/feedpropertiesdialog.ui ui/tagpropertieswidget.ui)

kde4_add_library(krss SHARED ${krss_LIB_SRCS})

if(KRSS_ENABLE_PROTOBUF_SERIALIZER)
endif(KRSS_ENABLE_PROTOBUF_SERIALIZER)

target_link_libraries(krss ${KDEPIMLIBS_SYNDICATION_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KRSS_EXTRA_LIBS})

if (Nepomuk_FOUND)
    target_link_libraries(krss ${NEPOMUK_LIBRARIES} ${SOPRANO_LIBRARIES})
endif (Nepomuk_FOUND)

set_target_properties(krss PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krss DESTINATION ${LIB_INSTALL_DIR})
install( FILES ${krss_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/krss )
install( FILES ${krss_UI_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/krss/ui )
install( FILES ${krss_PRIVATE_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/krss/private )
