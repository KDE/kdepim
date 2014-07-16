project(mail)

set(mail_SRCS
    mailplugin.cpp
    message.cpp
    composer.cpp
    error.cpp
    receivermodel.cpp
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/messagecomposer
    ${CMAKE_BINARY_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/messagecore
    ${CMAKE_SOURCE_DIR}/libkdepim/
)

add_library(mailplugin SHARED ${mail_SRCS})

target_link_libraries(mailplugin
    Qt5::Declarative
    KF5::AkonadiCore
    KF5::Mime
    messagecomposer
    kdepim
)

install(TARGETS mailplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/pim/mail)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/pim/mail)
