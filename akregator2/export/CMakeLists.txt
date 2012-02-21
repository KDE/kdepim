include_directories( ${CMAKE_SOURCE_DIR}/akregator2/export/metakit/include
                     ${CMAKE_SOURCE_DIR}
                     ${KDE4_INCLUDE_DIR}
                     ${QT_INCLUDES}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_BINARY_DIR}/akregator2
                     ${Boost_INCLUDE_DIR} )

set(metakit_SRCS
                metakit/src/column.cpp
                metakit/src/custom.cpp
                metakit/src/derived.cpp
                metakit/src/field.cpp
                metakit/src/fileio.cpp
                metakit/src/format.cpp
                metakit/src/handler.cpp
                metakit/src/persist.cpp
                metakit/src/remap.cpp
                metakit/src/std.cpp
                metakit/src/store.cpp
                metakit/src/string.cpp
                metakit/src/table.cpp
                metakit/src/univ.cpp
                metakit/src/view.cpp
                metakit/src/viewx.cpp
                )

set(akregator2storageexporter_SRCS
    ${metakit_SRCS}
    akregator2storageexporter.cpp
    storagemk4impl.cpp
    feedstoragemk4impl.cpp
)

kde4_add_executable(akregator2storageexporter NOGUI ${akregator2storageexporter_SRCS})

target_link_libraries(akregator2storageexporter
                      ${KDE4_KDECORE_LIBS}
                      ${KDEPIMLIBS_SYNDICATION_LIBS})

install(TARGETS akregator2storageexporter ${INSTALL_TARGETS_DEFAULT_ARGS})

