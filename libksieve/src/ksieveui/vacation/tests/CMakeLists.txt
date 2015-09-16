include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
        )


set(vacation_multi_server_SRCS
  main.cpp
)

add_executable(vacationmultiservertest ${vacation_multi_server_SRCS} )
target_link_libraries(vacationmultiservertest  ksieveui ksieve)

