if(KDE4_BUILD_TESTS)
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

add_subdirectory(tests)

set(krssresource_LIB_SRCS
    rssbackendjobs.cpp
    rssresourcebase.cpp
    rssresourcebasejobs.cpp
    importopmljob.cpp
    exportopmljob.cpp
    importitemsjob.cpp
    itemimportreader.cpp
    batchitemmodifyjob.cpp
    rssitemsync.cpp
    util.cpp
    opmlparser.cpp
)

set(krssresource_LIB_HDRS
    krssresource_export.h
    rssbackendjobs.h
    rssresourcebase.h
    opmlparser.h
)

qt4_add_dbus_adaptor(krssresource_LIB_SRCS org.kde.krss.xml rssresourcebase.h KRssResource::RssResourceBase )

kde4_add_library(krssresource SHARED ${krssresource_LIB_SRCS})
target_link_libraries(krssresource krss ${KDEPIMLIBS_SYNDICATION_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS}
                      ${KDEPIMLIBS_AKONADI_LIBS})

set_target_properties(krssresource PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS krssresource DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${krssresource_LIB_HDRS} DESTINATION ${INCLUDE_INSTALL_DIR}/krssresource)
