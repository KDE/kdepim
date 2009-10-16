INCLUDE_DIRECTORIES(BEFORE SYSTEM   ${CMAKE_BINARY_DIR}/standalone/demo
				    ${CMAKE_BINARY_DIR}/standalone)

KDE4_ADD_EXECUTABLE(standalone-demo main.cpp)

TARGET_LINK_LIBRARIES(standalone-demo	stickynotes
					stickynotes-standalone
                                        ${QT_QTCORE_LIBRARY}
                                        ${QT_QTGUI_LIBRARY}
                                        )

