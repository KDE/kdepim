set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${Boost_INCLUDE_DIR} )

########### next target ###############

set(testaddresseelineedit_SRCS testaddresseelineedit.cpp )

kde4_add_executable(testaddresseelineedit TEST ${testaddresseelineedit_SRCS})

target_link_libraries(testaddresseelineedit  kdepim ${KDE4_KDEUI_LIBS})

########### next target ###############

set(testdateedit_SRCS testdateedit.cpp )


kde4_add_executable(testdateedit TEST ${testdateedit_SRCS})

target_link_libraries(testdateedit  kdepim ${KDE4_KDEUI_LIBS})

########### next target ###############

set(testldapclient_SRCS testldapclient.cpp )

kde4_add_executable(testldapclient TEST ${testldapclient_SRCS})

target_link_libraries(testldapclient ${KDE4_KDEUI_LIBS} kdepim)
