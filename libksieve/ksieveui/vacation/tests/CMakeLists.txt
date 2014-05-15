include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
        )


set(vacation_multi_server_SRCS
  main.cpp
)

add_executable(vacationmultiservertest ${vacation_multi_server_SRCS} )
TARGET_LINK_LIBRARIES(vacationmultiservertest  ksieveui ksieve)

