kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabc2mutt_SRCS kabc2mutt.cpp main.cpp )

kde4_automoc(${kabc2mutt_SRCS})

kde4_add_executable(kabc2mutt ${kabc2mutt_SRCS})

target_link_libraries(kabc2mutt  ${QT_AND_KDECORE_LIBS} kabc )

install_targets(/bin kabc2mutt )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(includedir)/kabc $(all_includes)
#
#METASOURCES = AUTO
#
#bin_PROGRAMS = kabc2mutt
#
#kabc2mutt_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kabc2mutt_LDADD = -lkabc $(LIB_KDECORE)
#kabc2mutt_SOURCES = kabc2mutt.cpp main.cpp
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc2mutt.pot
#
