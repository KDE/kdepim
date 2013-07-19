project(xmlsieveparsing)

include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/scriptsparsing/
        )


set(xmlsieveparsing_SRCS
  main.cpp
)

KDE4_ADD_EXECUTABLE(xmlsieveparsing ${xmlsieveparsing_SRCS} )
TARGET_LINK_LIBRARIES(xmlsieveparsing ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ksieveui ksieve)

