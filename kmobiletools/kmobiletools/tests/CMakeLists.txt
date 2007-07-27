########### next target ###############

set(enginexp_SRCS enginexp.cpp )
kde4_add_executable(enginexptest ${enginexp_SRCS})
target_link_libraries(enginexptest ${KDE4_KDECORE_LIBS} kmobiletools)
