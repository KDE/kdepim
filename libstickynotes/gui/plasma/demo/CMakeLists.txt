INCLUDE_DIRECTORIES(BEFORE SYSTEM ${CMAKE_BINARY_DIR}/plasma/demo)

SET(PLASMADEMO_SRCS	main.cpp
			noteserver.cpp
			noteconnection.cpp)

SET(PLASMADEMO_HDRS	noteserver.h
			noteconnection.h)

SET(PLASMADEMO_MOC_HDRS		noteserver.h
				noteconnection.h)

QT4_WRAP_CPP(PLASMADEMO_MOC_SRCS ${PLASMADEMO_MOC_HDRS})
ADD_CUSTOM_TARGET(plasma_demo_qt4 DEPENDS ${PLASMADEMO_MOC_SRCS})

ADD_EXECUTABLE(plasma_demo ${PLASMADEMO_SRCS})
ADD_DEPENDENCIES(plasma_demo plasma_demo_qt4)

TARGET_LINK_LIBRARIES(plasma_demo   stickynotes
				    ${QT_LIBRARIES})

