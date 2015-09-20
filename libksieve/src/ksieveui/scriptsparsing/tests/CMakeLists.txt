include_directories(${libksieveui_SOURCE_DIR}
        ${libksieve_SOURCE_DIR}/ksieveui
        ${libksieve_SOURCE_DIR}/ksieveui/scriptsparsing/
        )


set(xmlsieveparsing_SRCS
  main.cpp
)

add_executable(xmlsieveparsing ${xmlsieveparsing_SRCS} )
target_link_libraries(xmlsieveparsing   ksieveui ksieve)

