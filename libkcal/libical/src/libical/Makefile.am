#======================================================================
#  FILE: Makefile.am
#  CREATOR: eric 
#  
#  $Id$
#
#
# (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of either: 
#
#    The LGPL as published by the Free Software Foundation, version
#    2.1, available at: http://www.fsf.org/copyleft/lesser.html
#
#  Or:
#
#    The Mozilla Public License Version 1.0. You may obtain a copy of
#    the License at http://www.mozilla.org/MPL/
#
#  The original code is icalcomponent.c
#
#======================================================================


DESIGNDATA = $(top_srcdir)/libkcal/libical/design-data
ICALSCRIPTS = $(top_srcdir)/libkcal/libical/scripts

noinst_LTLIBRARIES = libical.la

AM_YFLAGS = -d -v -t -pical_yy
AM_LFLAGS = -Pical_yy -olex.yy.c

INCLUDES =			\
	-I$(top_builddir)/libkcal/libical	\
	-I$(top_srcdir)/libkcal/libical/src	\
	-I$(top_builddir)/libkcal/libical/src	\
	-I$(srcdir)

libical_la_LDFLAGS = -no-undefined

# Order is important!
libical_la_SOURCES =		\
	icalderivedparameter.c	\
	icalrecur.c		\
	icalattendee.c		\
	icalcomponent.c		\
	icalenums.c		\
	icalerror.c		\
	icallexer.c		\
	icalmemory.c		\
	icalmime.c		\
	icalparameter.c		\
	icalparser.c		\
	icalderivedproperty.c	\
	icalproperty.c		\
	icalrestriction.c	\
	icaltime.c		\
	icalduration.c		\
	icalperiod.c		\
	icaltypes.c		\
	icalvalue.c		\
	icalderivedvalue.c	\
	icalyacc.c		\
	pvl.c			\
	sspm.c			\
	vsnprintf.c		\
	icallangbind.c

#	icalyacc.y		
#	icallexer.l		

noinst_HEADERS =  ical.h \
	icalderivedparameter.h	\
	icalrecur.h		\
	icalattendee.h		\
	icalcomponent.h		\
	icalenums.h		\
	icalerror.h		\
	icalmemory.h		\
	icalmime.h		\
	icalparameter.h		\
	icalparameterimpl.h	\
	icalparser.h		\
	icalderivedproperty.h	\
	icalproperty.h		\
	icalrestriction.h	\
	icaltime.h		\
	icalduration.h		\
	icalperiod.h		\
	icaltypes.h		\
	icalvalue.h		\
	icalvalueimpl.h		\
	icalderivedvalue.h	\
	icalyacc.h		\
	pvl.h			\
	sspm.h			\
	icallangbind.h

# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering. It
# is required to make the combined header ical.h properly
COMBINEDHEADERS =					\
	$(srcdir)/icalversion.h	\
	$(srcdir)/icaltime.h		\
	$(srcdir)/icalduration.h	\
	$(srcdir)/icalperiod.h		\
	$(srcdir)/icalenums.h		\
	$(srcdir)/icaltypes.h		\
	$(srcdir)/icalrecur.h		\
	icalderivedvalue.h		\
	icalderivedparameter.h		\
	icalderivedproperty.h		\
	$(srcdir)/icalvalue.h		\
	$(srcdir)/icalparameter.h	\
	$(srcdir)/icalproperty.h	\
	$(srcdir)/icalattendee.h	\
	$(srcdir)/pvl.h			\
	$(srcdir)/icalcomponent.h	\
	$(srcdir)/icalparser.h		\
	$(srcdir)/icalmemory.h		\
	$(srcdir)/icalerror.h		\
	$(srcdir)/icalrestriction.h	\
	$(srcdir)/sspm.h		\
	$(srcdir)/icalmime.h 		\
	$(srcdir)/icallangbind.h	

ical.h: $(COMBINEDHEADERS)
	cat $(COMBINEDHEADERS)				\
	| egrep -v "#include.*\"ical"			\
	| egrep -v "#include.*\"pvl\.h\"" > ical.h

# parameters

PARAMETERDEPS =	\
	$(ICALSCRIPTS)/mkderivedparameters.pl \
	$(DESIGNDATA)/parameters.csv	\
	icalderivedparameter.c.in \
	icalderivedparameter.h.in

icalderivedparameter.h: $(PARAMETERDEPS) 
	$(PERL) -I $(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedparameters.pl -i $(srcdir)/icalderivedparameter.h.in -h $(DESIGNDATA)/parameters.csv > icalderivedparameter.h 

icalderivedparameter.c: $(PARAMETERDEPS) icalparameter.h
	$(PERL) -I $(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedparameters.pl -i $(srcdir)/icalderivedparameter.c.in -c $(DESIGNDATA)/parameters.csv > icalderivedparameter.c


# properties

PROPERTYDEPS =					\
	$(ICALSCRIPTS)/mkderivedproperties.pl	\
	$(DESIGNDATA)/properties.csv		\
	$(DESIGNDATA)/value-types.csv		\
	icalderivedproperty.c.in		\
	icalderivedproperty.h.in


icalderivedproperty.h: $(PROPERTYDEPS)
	$(PERL)  -I$(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedproperties.pl \
		-i $(srcdir)/icalderivedproperty.h.in -h $(DESIGNDATA)/properties.csv\
		${DESIGNDATA}/value-types.csv > icalderivedproperty.h 

icalderivedproperty.c: $(PROPERTYDEPS) icalproperty.h 
	$(PERL)  -I$(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedproperties.pl \
		-i $(srcdir)/icalderivedproperty.c.in -c $(DESIGNDATA)/properties.csv \
		${DESIGNDATA}/value-types.csv > icalderivedproperty.c

# restrictions

RESTRICTIONDEPS =				\
	$(ICALSCRIPTS)/mkrestrictiontable.pl	\
	$(DESIGNDATA)/restrictions.csv		\
	icalrestriction.c.in

icalrestriction.c: $(RESTRICTIONDEPS)
	$(PERL) $(ICALSCRIPTS)/mkrestrictiontable.pl  -i $(srcdir)/icalrestriction.c.in \
		$(DESIGNDATA)/restrictions.csv > icalrestriction.c

# values

VALUEDEPS =					\
	$(ICALSCRIPTS)/mkderivedvalues.pl  	\
	$(DESIGNDATA)/value-types.csv		\
	icalderivedvalue.c.in				\
	icalderivedvalue.h.in

icalderivedvalue.h: $(VALUEDEPS)
	$(PERL) -I$(ICALSCRIPTS)  $(ICALSCRIPTS)/mkderivedvalues.pl \
	         -i $(srcdir)/icalderivedvalue.h.in -h $(DESIGNDATA)/value-types.csv > icalderivedvalue.h

icalderivedvalue.c: $(VALUEDEPS) icalderivedvalue.h
	$(PERL)  -I$(ICALSCRIPTS) $(ICALSCRIPTS)/mkderivedvalues.pl \
		-i $(srcdir)/icalderivedvalue.c.in -c $(DESIGNDATA)/value-types.csv > icalderivedvalue.c

# housekeeping
CONFIG_CLEAN_FILES = y.output

EXTRA_DIST =			\
	icalderivedparameter.c.in \
	icalderivedparameter.h.in \
	icalderivedproperty.c.in \
	icalderivedproperty.h.in \
	icalrestriction.c.in	\
	icalderivedvalue.c.in	\
	icalderivedvalue.h.in	\
	icalversion.h.in	\
	icallexer.c		\
	icalyacc.c

icalattendee.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalcomponent.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalderivedparameter.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalderivedproperty.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalderivedvalue.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalduration.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icallangbind.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icallexer.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalmime.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalparameter.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalparser.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalproperty.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalrestriction.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icaltime.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalvalue.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
icalyacc.lo: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h

