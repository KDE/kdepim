
########### next target ###############
if(KDE4_BUILD_TESTS)
set(dataprovidertest_SRCS dataprovidertest.cpp )

kde4_automoc(${dataprovidertest_SRCS})

kde4_add_executable(dataprovidertest ${dataprovidertest_SRCS})

target_link_libraries(dataprovidertest  qgpgme ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

