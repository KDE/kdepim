kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )
add_subdirectory(akregator)
add_subdirectory(api)
add_subdirectory(common)
add_subdirectory(kaddressbook)
add_subdirectory(kalarm)
add_subdirectory(karm)
add_subdirectory(kleopatra)
add_subdirectory(kmail)
add_subdirectory(knode)
add_subdirectory(knotes)
add_subdirectory(konsolekalendar)
add_subdirectory(kontact)
add_subdirectory(korganizer)
add_subdirectory(korn)
add_subdirectory(ktnef)
add_subdirectory(kwatchgnupg)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#KDE_LANG = en
#KDE_DOCS = AUTO
#SUBDIRS = $(AUTODIRS)
#
