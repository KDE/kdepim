include_directories(${libksieve_SOURCE_DIR}
        ${libksieve_SOURCE_DIR}/src/ksieveui
        )


set(vacation_multi_server_SRCS
  main.cpp
)

add_executable(vacationmultiservertest ${vacation_multi_server_SRCS} )
target_link_libraries(vacationmultiservertest  KF5::KSieveUi KF5::KSieve)

