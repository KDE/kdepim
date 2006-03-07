kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkcal/libical/src ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


set(PROPERTYDEPS 
	${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedproperties.pl 
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/properties.csv	
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h.in 
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c.in 
)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedproperties.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h.in -h ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h   DEPENDS  ${PROPERTYDEPS} )


add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedproperties.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c.in -c ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c   DEPENDS  ${PROPERTYDEPS} ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h )


macro_additional_clean_files( ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h )


set(PARAMETERDEPS 
	${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedproperties.pl 
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv 
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c.in
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h.in
)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedparameters.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h.in -h ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h   DEPENDS  ${PARAMETERDEPS} )

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedparameters.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c.in -c ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/parameters.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.c   DEPENDS  ${PARAMETERDEPS} ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedparameter.h )

macro_additional_clean_files( 
	{CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.c 
	{CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedproperty.h 
)


set(RESTRICTIONDEPS 
	${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkrestrictiontable.pl
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/restrictions.csv
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c.in
)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkrestrictiontable.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c.in -h ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/restrictions.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c   DEPENDS  ${RESTRICTIONDEPS} )


macro_additional_clean_files( ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalrestriction.c)

set(VALUEDEPS
	${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedvalues.pl
	${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c.in
	${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h.in
)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedvalues.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h.in -h ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h   DEPENDS  ${VALUEDEPS} )


add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c COMMAND ${PERL_EXECUTABLE} ARGS -I${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/ ${CMAKE_SOURCE_DIR}/libkcal/libical/scripts/mkderivedvalues.pl -i ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c.in -c ${CMAKE_SOURCE_DIR}/libkcal/libical/design-data/value-types.csv > ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c   DEPENDS  ${VALUEDEPS} ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h )

macro_additional_clean_files(${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.h ${CMAKE_SOURCE_DIR}/libkcal/libical/src/libical/icalderivedvalue.c )

########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##======================================================================
##  FILE: Makefile.am
##  CREATOR: eric 
##  
##  $Id$
##
##
## (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of either: 
##
##    The LGPL as published by the Free Software Foundation, version
##    2.1, available at: http://www.fsf.org/copyleft/lesser.html
##
##  Or:
##
##    The Mozilla Public License Version 1.0. You may obtain a copy of
##    the License at http://www.mozilla.org/MPL/
##
##  The original code is icalcomponent.c
##
##======================================================================
#
#
#DESIGNDATA = $(srcdir)/../../design-data
#ICALSCRIPTS = $(srcdir)/../../scripts
#
#noinst_LTLIBRARIES = libical.la
#
#AM_CPPFLAGS = -DPACKAGE_DATA_DIR=\""$(kde_datadir)/libical"\"
#
#AM_YFLAGS = -d -v -t -pical_yy
#AM_LFLAGS = -Pical_yy -olex.yy.c
#
#INCLUDES =                     \
#       -I$(top_builddir)/libkcal/libical       \
#       -I$(top_srcdir)/libkcal/libical/src     \
#       -I$(top_builddir)/libkcal/libical/src   \
#       -I$(srcdir)
#
#libical_la_LDFLAGS = -no-undefined
#
## Order is important!
#libical_la_SOURCES =		\
#	caldate.c		\
#	icalarray.c		\
#	icalattach.c		\
#	icalderivedparameter.c	\
#	icalrecur.c		\
#	icalrestriction.c	\
#	icalcomponent.c		\
#	icalenums.c		\
#	icalerror.c		\
#	icalmemory.c		\
#	icalmime.c		\
#	icalparameter.c		\
#	icalparser.c		\
#	icalderivedproperty.c	\
#	icalproperty.c		\
#	icaltime.c		\
#	icalduration.c		\
#	icalperiod.c		\
#	icaltimezone.c		\
#	icaltypes.c		\
#	icalvalue.c		\
#	icalderivedvalue.c	\
#	pvl.c			\
#	sspm.c			\
#	vsnprintf.c		\
#	icallangbind.c
#
#
#noinst_HEADERS = ical.h		\
#	astime.h		\
#	icalarray.h		\
#	icalattach.h		\
#	icalattachimpl.h	\
#	icalderivedparameter.h	\
#	icalrecur.h		\
#	icalcomponent.h		\
#	icalenums.h		\
#	icalerror.h		\
#	icalmemory.h		\
#	icalmime.h		\
#	icalparameter.h		\
#	icalparameterimpl.h	\
#	icalparser.h		\
#	icalderivedproperty.h	\
#	icalproperty.h		\
#	icaltime.h		\
#	icalduration.h		\
#	icalperiod.h		\
#	icaltimezone.h		\
#	icaltypes.h		\
#	icalvalue.h		\
#	icalvalueimpl.h		\
#	icalderivedvalue.h	\
#	pvl.h			\
#	sspm.h			\
#	icallangbind.h
#
## ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering. It
## is required to make the combined header ical.h properly
#COMBINEDHEADERS =                       \
#	$(srcdir)/icalversion.h         \
#	$(srcdir)/icaltime.h		\
#	$(srcdir)/icalduration.h	\
#	$(srcdir)/icalperiod.h		\
#	$(srcdir)/icalenums.h		\
#	$(srcdir)/icaltypes.h		\
#	$(srcdir)/icalrecur.h		\
#	$(srcdir)/icalattach.h		\
#	icalderivedvalue.h		\
#	icalderivedparameter.h		\
#	$(srcdir)/icalvalue.h		\
#	$(srcdir)/icalparameter.h	\
#	icalderivedproperty.h		\
#	$(srcdir)/icalproperty.h	\
#	$(srcdir)/pvl.h			\
#	$(srcdir)/icalarray.h		\
#	$(srcdir)/icalcomponent.h	\
#	$(srcdir)/icaltimezone.h	\
#	$(srcdir)/icalparser.h		\
#	$(srcdir)/icalmemory.h		\
#	$(srcdir)/icalerror.h		\
#	$(srcdir)/icalrestriction.h	\
#	$(srcdir)/sspm.h		\
#	$(srcdir)/icalmime.h		\
#	$(srcdir)/icallangbind.h
#
#ical.h: $(COMBINEDHEADERS)
#	echo '#ifdef __cplusplus'	  >  ical.h
#	echo 'extern "C" {'		  >> ical.h
#	echo '#endif'			  >> ical.h
#	echo '/*'			  >> ical.h
#	echo ' $$''Id''$$'		  >> ical.h
#	echo '*/'			  >> ical.h
#	cat $(COMBINEDHEADERS)				\
#	| egrep -v "#include.*\"ical"			\
#	| egrep -v "#include.*\"config"			\
#	| egrep -v "#include.*\"pvl\.h\""		\
#	| egrep -v '\$$(Id|Locker): .+\$$'>> ical.h
#	echo '#ifdef __cplusplus'	  >> ical.h
#	echo '}'			  >> ical.h
#	echo '#endif'			  >> ical.h
#
## parameters
#
#PARAMETERDEPS =	\
#	$(ICALSCRIPTS)/mkderivedparameters.pl \
#	$(DESIGNDATA)/parameters.csv	\
#	icalderivedparameter.c.in \
#	icalderivedparameter.h.in
#
#icalderivedparameter.h: $(PARAMETERDEPS) 
#	$(PERL) -I $(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedparameters.pl -i $(srcdir)/icalderivedparameter.h.in -h $(DESIGNDATA)/parameters.csv > icalderivedparameter.h  || rm -f icalderivedparameter.h
#
#
#icalderivedparameter.c: $(PARAMETERDEPS) icalparameter.h
#	$(PERL) -I $(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedparameters.pl -i $(srcdir)/icalderivedparameter.c.in -c $(DESIGNDATA)/parameters.csv > icalderivedparameter.c || rm -f icalderivedparameter.c
#
#
## properties
#
#PROPERTYDEPS =					\
#	$(ICALSCRIPTS)/mkderivedproperties.pl	\
#	$(DESIGNDATA)/properties.csv		\
#	$(DESIGNDATA)/value-types.csv		\
#	icalderivedproperty.c.in		\
#	icalderivedproperty.h.in
#
#
#icalderivedproperty.h: $(PROPERTYDEPS)
#	$(PERL)  -I$(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedproperties.pl \
#		-i $(srcdir)/icalderivedproperty.h.in -h $(DESIGNDATA)/properties.csv\
#		${DESIGNDATA}/value-types.csv > icalderivedproperty.h  || rm -f icalderivedproperty.h
#
#icalderivedproperty.c: $(PROPERTYDEPS) icalproperty.h 
#	$(PERL)  -I$(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedproperties.pl \
#		-i $(srcdir)/icalderivedproperty.c.in -c $(DESIGNDATA)/properties.csv \
#		${DESIGNDATA}/value-types.csv > icalderivedproperty.c || rm -f icalderivedproperty.c
#
## restrictions
#
#RESTRICTIONDEPS =				\
#	$(ICALSCRIPTS)/mkrestrictiontable.pl	\
#	$(DESIGNDATA)/restrictions.csv		\
#	icalrestriction.c.in
#
#icalrestriction.c: $(RESTRICTIONDEPS)
#	$(PERL) $(ICALSCRIPTS)/mkrestrictiontable.pl  -i $(srcdir)/icalrestriction.c.in \
#		$(DESIGNDATA)/restrictions.csv > icalrestriction.c || rm -f icalrestriction.c
#
## values
#
#VALUEDEPS =					\
#	$(ICALSCRIPTS)/mkderivedvalues.pl  	\
#	$(DESIGNDATA)/value-types.csv		\
#	icalderivedvalue.c.in				\
#	icalderivedvalue.h.in
#
#icalderivedvalue.h: $(VALUEDEPS)
#	$(PERL) -I$(ICALSCRIPTS)  $(ICALSCRIPTS)/mkderivedvalues.pl \
#	         -i $(srcdir)/icalderivedvalue.h.in -h $(DESIGNDATA)/value-types.csv > icalderivedvalue.h || rm -f icalderivedvalue.h
#
#icalderivedvalue.c: $(VALUEDEPS) icalderivedvalue.h
#	$(PERL)  -I$(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedvalues.pl \
#		-i $(srcdir)/icalderivedvalue.c.in -c $(DESIGNDATA)/value-types.csv > icalderivedvalue.c || rm -f icalderivedvalue.c
#
## housekeeping
#CLEANFILES = y.output  \
#	icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h ical.h \
#	icalderivedvalue.c icalderivedproperty.c icalderivedparameter.c \
#	icalrestriction.c
#
#EXTRA_DIST =			\
#	icalderivedparameter.c.in \
#	icalderivedparameter.h.in \
#	icalderivedproperty.c.in \
#	icalderivedproperty.h.in \
#	icalrestriction.c.in	\
#	icalderivedvalue.c.in	\
#	icalderivedvalue.h.in	\
#	icalversion.h.in	
#
#
#icalcomponent.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalderivedparameter.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalderivedproperty.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalderivedvalue.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalduration.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icallangbind.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalmime.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalparameter.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalparser.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalproperty.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalrestriction.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icaltime.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
#icalvalue.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
