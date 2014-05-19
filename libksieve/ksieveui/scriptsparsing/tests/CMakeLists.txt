project(xmlsieveparsing)

include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/scriptsparsing/
        )


set(xmlsieveparsing_SRCS
  main.cpp
)

add_executable(xmlsieveparsing ${xmlsieveparsing_SRCS} )
TARGET_LINK_LIBRARIES(xmlsieveparsing   ksieveui ksieve)

