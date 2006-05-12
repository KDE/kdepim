



########### next target ###############

set(copycluster_SRCS copycluster.c )

kde4_automoc(${copycluster_SRCS})

kde4_add_executable(copycluster ${copycluster_SRCS})

target_link_libraries(copycluster  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(regression_SRCS 
   regression.c 
   regression.h 
   regression-component.c 
   regression-classify.c 
   regression-utils.c 
   regression-recur.c 
   regression-storage.c )

kde4_automoc(${regression_SRCS})

kde4_add_executable(regression ${regression_SRCS})

target_link_libraries(regression  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(parser_SRCS icaltestparser.c )

kde4_automoc(${parser_SRCS})

kde4_add_executable(parser ${parser_SRCS})

target_link_libraries(parser  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(stow_SRCS stow.c )

kde4_automoc(${stow_SRCS})

kde4_add_executable(stow ${stow_SRCS})

target_link_libraries(stow  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(recur_SRCS recur.c )

kde4_automoc(${recur_SRCS})

kde4_add_executable(recur ${recur_SRCS})

target_link_libraries(recur  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(testmime_SRCS testmime.c )

kde4_automoc(${testmime_SRCS})

kde4_add_executable(testmime ${testmime_SRCS})

target_link_libraries(testmime  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(testvcal_SRCS testvcal.c )

kde4_automoc(${testvcal_SRCS})

kde4_add_executable(testvcal ${testvcal_SRCS})

target_link_libraries(testvcal  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(process_SRCS process.c )

kde4_automoc(${process_SRCS})

kde4_add_executable(process ${process_SRCS})

target_link_libraries(process  ${KDE4_KDECORE_LIBS} )


########### install files ###############





#original Makefile.am contents follow:

#noinst_PROGRAMS = copycluster regression parser stow recur testmime testvcal process
#
#LDADD =  ../libicalss/.libs/libicalss.a ../libicalvcal/.libs/libicalvcal.a ../libical/.libs/libical.a 
#
##LIBS = @PTHREAD_LIBS@
#
#INCLUDES = 				\
#       -I. 				\
#       -I../libicalvcal 		\
#       -I$(srcdir)/../libicalvcal 	\
#       -I$(srcdir) 			\
#       -I../libical 			\
#       -I$(srcdir)/../libical 		\
#       -I../libicalss 			\
#       -I$(srcdir)/../libicalss 
#
#TESTS=regression
#copycluster_SOURCES = copycluster.c
#regression_SOURCES = 		\
#	regression.c		\
#	regression.h		\
#	regression-component.c	\
#	regression-classify.c	\
#	regression-utils.c	\
#	regression-recur.c	\
#	regression-storage.c
#
#parser_SOURCES = icaltestparser.c
#stow_SOURCES = stow.c
#testvcal_SOURCES = testvcal.c
#recur_SOURCES = recur.c 
#testmime_SOURCES = testmime.c
#process_SOURCES = process.c
