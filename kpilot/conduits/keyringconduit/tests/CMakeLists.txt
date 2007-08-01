include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_BINARY_DIR}
)

set(tests_SRCS
	main.cc
)

kde4_add_executable(testje ${tests_SRCS})
target_link_libraries(testje ${QT_QTCORE_LIBRARY} kpilot keyring)
