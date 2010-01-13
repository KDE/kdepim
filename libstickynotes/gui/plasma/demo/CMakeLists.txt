INCLUDE_DIRECTORIES(BEFORE SYSTEM ${CMAKE_BINARY_DIR}/plasma/demo)

SET(PLASMADEMO_SRCS	main.cpp
			noteserver.cpp
			noteconnection.cpp)


KDE4_ADD_EXECUTABLE(plasma_demo ${PLASMADEMO_SRCS})
ADD_DEPENDENCIES(plasma_demo plasma_demo_qt4)

TARGET_LINK_LIBRARIES(plasma_demo   stickynotes
				    ${QT_QTCORE_LIBRARY}
				    ${QT_QTGUI_LIBRARY}
				    ${QT_QTNETWORK_LIBRARY}
                                    )

