project(mail)

set(mail_SRCS
    mailplugin.cpp
    folderlist.cpp
    message.cpp
    composer.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_library(mailplugin SHARED ${mail_SRCS})

target_link_libraries(mailplugin
    Qt5::Declarative
    KF5::AkonadiCore
    KF5::Mime
)

install(TARGETS mailplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/pim/mail)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/pim/mail)
