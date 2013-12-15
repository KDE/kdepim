include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
        )


set(vacation_multi_server_SRCS
  main.cpp
)

KDE4_ADD_EXECUTABLE(vacationmultiservertest ${vacation_multi_server_SRCS} )
TARGET_LINK_LIBRARIES(vacationmultiservertest ${KDE4_KDEUI_LIBS} ksieveui ksieve)

