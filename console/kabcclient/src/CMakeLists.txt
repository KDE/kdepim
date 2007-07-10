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


kde4_add_executable(kabcclient NOGUI ${kabcclient_SRCS})

target_link_libraries(kabcclient ${KDE4_KABC_LIBS} ${KDE4_KDEUI_LIBS} )

install(TARGETS kabcclient DESTINATION ${BIN_INSTALL_DIR})
#note: kabcclient acts as kabc2mutt invoked with that name
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/kabcclient DESTINATION ${BIN_INSTALL_DIR} RENAME kabc2mutt)
