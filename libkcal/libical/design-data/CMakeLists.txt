kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#EXTRA_DIST =\
# properties.csv \
# parameters.csv \
# restrictions.csv \
# status.txt \
# value-types.csv
