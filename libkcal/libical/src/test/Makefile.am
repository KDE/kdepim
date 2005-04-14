noinst_PROGRAMS = copycluster regression parser stow recur testmime testvcal process

LDADD =  ../libicalss/.libs/libicalss.a ../libicalvcal/.libs/libicalvcal.a ../libical/.libs/libical.a 

#LIBS = @PTHREAD_LIBS@

INCLUDES = 				\
       -I. 				\
       -I../libicalvcal 		\
       -I$(srcdir)/../libicalvcal 	\
       -I$(srcdir) 			\
       -I../libical 			\
       -I$(srcdir)/../libical 		\
       -I../libicalss 			\
       -I$(srcdir)/../libicalss 

TESTS=regression
copycluster_SOURCES = copycluster.c
regression_SOURCES = 		\
	regression.c		\
	regression.h		\
	regression-component.c	\
	regression-classify.c	\
	regression-utils.c	\
	regression-recur.c	\
	regression-storage.c

parser_SOURCES = icaltestparser.c
stow_SOURCES = stow.c
testvcal_SOURCES = testvcal.c
recur_SOURCES = recur.c 
testmime_SOURCES = testmime.c
process_SOURCES = process.c
