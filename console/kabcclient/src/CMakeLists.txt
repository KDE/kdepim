project(kabcclient)

add_subdirectory(examples) 

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kabcclient_SRCS
    main.cpp
    csvtemplate.cpp
    csvtemplatefactory.cpp
    formatfactory.cpp
    kabcclient.cpp
    inputformatimpls.cpp
    outputformatimpls.cpp)


# todo: more appropriate icon?
kde4_add_app_icon(kabcclient_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/office-address-book.png")

kde4_add_executable(kabcclient NOGUI ${kabcclient_SRCS})

target_link_libraries(kabcclient ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KDEUI_LIBS} )

install(TARGETS kabcclient ${INSTALL_TARGETS_DEFAULT_ARGS})

if(NOT WIN32)
#note: kabcclient acts as kabc2mutt invoked with that name
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/kabcclient DESTINATION ${BIN_INSTALL_DIR} RENAME kabc2mutt)
else()
kde4_add_executable(kabc2mutt NOGUI ${kabcclient_SRCS})
target_link_libraries(kabc2mutt ${KDEPIMLIBS_KABC_LIBS} ${KDE4_KDEUI_LIBS} )
install(TARGETS kabc2mutt ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()

