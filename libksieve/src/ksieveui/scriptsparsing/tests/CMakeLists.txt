include_directories(${libksieveui_SOURCE_DIR}
        ${libksieve_SOURCE_DIR}/src/ksieveui
        ${libksieve_SOURCE_DIR}/src/ksieveui/scriptsparsing/
        )


set(xmlsieveparsing_SRCS
  main.cpp
)

add_executable(xmlsieveparsing ${xmlsieveparsing_SRCS} )
target_link_libraries(xmlsieveparsing   KF5::KSieveUi KF5::KSieve)

