kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#KDE_LANG = en
#KDE_DOCS = AUTO
#
## $(srcdir)/translation.po: $(srcdir)/german_original.docbook
##	xml2pot $(srcdir)/german_original.docbook > german.pot
##	msgmerge -o $(srcdir)/translation.po $(srcdir)/translation.po german.pot
##	rm german.pot
#
