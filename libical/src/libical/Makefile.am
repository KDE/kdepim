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


DESIGNDATA = $(top_srcdir)/design-data
ICALSCRIPTS = $(top_srcdir)/scripts

lib_LTLIBRARIES = libical.la

AM_YFLAGS = -d -v -t -pyy
AM_LFLAGS = -Pyy

INCLUDES =			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/src	\
	-I$(srcdir)

libical_la_LDFLAGS = -version-info 0:0:0

# Order is important!
libical_la_SOURCES =		\
	icalderivedparameter.c	\
	icalderivedparameter.h	\
	icalrecur.c		\
	icalrecur.h		\
	icalattendee.h		\
	icalattendee.c		\
	icalcomponent.c		\
	icalcomponent.h		\
	icalenums.c		\
	icalenums.h		\
	icalerror.c		\
	icalerror.h		\
	icallexer.l		\
	icalmemory.c		\
	icalmemory.h		\
	icalmime.c		\
	icalmime.h		\
	icalparameter.c		\
	icalparameter.h		\
	icalparameterimpl.h	\
	icalparser.c		\
	icalparser.h		\
	icalderivedproperty.c	\
	icalderivedproperty.h	\
	icalproperty.c		\
	icalproperty.h		\
	icalrestriction.c	\
	icalrestriction.h	\
	icaltime.c		\
	icaltime.h		\
	icalduration.h		\
	icalduration.c		\
	icalperiod.h		\
	icalperiod.c		\
	icaltypes.c		\
	icaltypes.h		\
	icalvalue.c		\
	icalvalue.h		\
	icalvalueimpl.h		\
	icalderivedvalue.c	\
	icalderivedvalue.h	\
	icalyacc.h		\
	icalyacc.y		\
	pvl.c			\
	pvl.h			\
	sspm.c			\
	sspm.h			\
	vsnprintf.c		\
	icallangbind.h		\
	icallangbind.c

libicalincludedir = $(includedir)

libicalinclude_HEADERS =  ical.h 

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

icallexer.c : icalyacc.h

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

$(srcdir)/icalattendee.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalcomponent.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalderivedparameter.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalderivedproperty.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalderivedvalue.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalduration.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icallangbind.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icallexer.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalmime.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalparameter.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalparser.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalproperty.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalrestriction.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icaltime.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalvalue.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h
$(srcdir)/icalyacc.c: icalderivedvalue.h icalderivedproperty.h icalderivedparameter.h

