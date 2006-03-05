kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#EXTRA_DIST = \
#mkderivedcomponents.pl \
#mkderivedparameters.pl \
#mkderivedproperties.pl \
#mkderivedvalues.pl \
#mkparameterrestrictions.pl \
#mkrestrictionrecords.pl \
#mkrestrictiontable.pl \
#readvaluesfile.pl
