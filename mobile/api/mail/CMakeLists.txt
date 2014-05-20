project(mail)

set(mail_SRCS
    mailplugin.cpp
    folderlist.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

qt4_automoc( ${mail_SRCS})

kde4_add_library(mailplugin SHARED ${mail_SRCS})

target_link_libraries(mailplugin

    ${QT_QTCORE_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}

)

install(TARGETS mailplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/pim/mail)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/pim/mail)