DESIGNDATA = $(top_srcdir)/design-data
ICALSCRIPTS = $(top_srcdir)/scripts

lib_LTLIBRARIES = libical.la

YFLAGS = -d -v -t -pical_yy
LFLAGS = -Pical_yy
LEX_OUTPUT_ROOT = lex.ical_yy

all: ical.h

INCLUDES =			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/src	\
	-I$(top_builddir)/src	\
	-I$(srcdir)

libical_la_LDFLAGS = -version-info 0:0:0

libical_la_SOURCES =		\
	icalcomponent.c		\
	icalenums.c		\
	icalerror.c		\
	icallexer.l		\
	icalmemory.c		\
	icalmime.c		\
	icalparameter.c		\
	icalparser.c		\
	icalproperty.c		\
	icalrecur.c		\
	icalrestriction.c	\
	icaltime.c		\
	icaltypes.c		\
	icalvalue.c		\
	icalyacc.h		\
	icalyacc.y		\
	pvl.c			\
	sspm.c

libicalincludedir = $(includedir)

libicalinclude_HEADERS =  ical.h 

# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering. It
# is required to make the combined header ical.h properly
COMBINEDHEADERS=\
	icalversion.h \
	icaltime.h \
	icalenums.h \
	icaltypes.h \
	icalvalue.h \
	icalparameter.h \
	icalproperty.h \
	pvl.h \
	icalcomponent.h	\
	icalparser.h \
	icalmemory.h \
	icalerror.h \
	icalrestriction.h \
	icalrecur.h \
	sspm.h \
	icalmime.h 


BUILT_SOURCES =			\
	icalparameter.c		\
	icalparameter.h		\
	icalproperty.c		\
	icalproperty.h		\
	icalrestriction.c	\
	icalvalue.c		\
	icalvalue.h

#This is not srcdir!=builddir safe, and ical.h is in CVS anyway
#ical.h: $(COMBINEDHEADERS) $(BUILT_SOURCES)
#	cat $(COMBINEDHEADERS) | egrep -v "#include.*\"ical" \
#		| egrep -v "#include.*\"pvl\.h\"" > ical.h

icallexer.c : icalyacc.h


# parameters

PARAMETERDEPS =	\
	$(ICALSCRIPTS)/mkderivedparameters.pl \
	$(DESIGNDATA)/param-c-types.txt	\
	icalparameter.c.in \
	icalparameter.h.in

icalparameter.h: $(PARAMETERDEPS) 
	$(PERL) $(ICALSCRIPTS)/mkderivedparameters.pl -i icalparameter.h.in -h $(DESIGNDATA)/param-c-types.txt > icalparameter.newh \
	&& mv icalparameter.newh icalparameter.h 

icalparameter.c: $(PARAMETERDEPS) icalparameter.h
	$(PERL) $(ICALSCRIPTS)/mkderivedparameters.pl -i icalparameter.c.in -c $(DESIGNDATA)/param-c-types.txt > icalparameter.newc \
	&& mv icalparameter.newc icalparameter.c

# properties

PROPERTYDEPS =					\
	$(ICALSCRIPTS)/mkderivedproperties.pl	\
	$(DESIGNDATA)/prop-to-value.txt		\
	$(DESIGNDATA)/value-c-types.txt		\
	icalproperty.c.in			\
	icalproperty.h.in


icalproperty.h: $(PROPERTYDEPS)
	$(PERL) $(ICALSCRIPTS)/mkderivedproperties.pl -i icalproperty.h.in -h \
		$(DESIGNDATA)/prop-to-value.txt \
		${DESIGNDATA}/value-c-types.txt > icalproperty.h 

icalproperty.c: $(PROPERTYDEPS) icalproperty.h 
	$(PERL) $(ICALSCRIPTS)/mkderivedproperties.pl -i icalproperty.c.in -c \
		$(DESIGNDATA)/prop-to-value.txt \
		${DESIGNDATA}/value-c-types.txt > icalproperty.c

# restrictions

RESTRICTIONDEPS =				\
	$(ICALSCRIPTS)/mkrestrictiontable.pl	\
	$(DESIGNDATA)/restrictions.csv		\
	icalrestriction.c.in

icalrestriction.c: $(RESTRICTIONDEPS)
	$(PERL) $(ICALSCRIPTS)/mkrestrictiontable.pl  -i icalrestriction.c.in \
		$(DESIGNDATA)/restrictions.csv > icalrestriction.c

# values

VALUEDEPS =					\
	$(ICALSCRIPTS)/mkderivedvalues.pl  	\
	$(DESIGNDATA)/value-c-types.txt		\
	icalvalue.c.in				\
	icalvalue.h.in

icalvalue.h: $(VALUEDEPS)
	$(PERL) $(ICALSCRIPTS)/mkderivedvalues.pl -i icalvalue.h.in -h \
		$(DESIGNDATA)/value-c-types.txt > icalvalue.h

icalvalue.c: $(VALUEDEPS) icalvalue.h
	$(PERL) $(ICALSCRIPTS)/mkderivedvalues.pl  -i icalvalue.c.in -c \
		$(DESIGNDATA)/value-c-types.txt > icalvalue.c



# housekeeping
CONFIG_CLEAN_FILES = y.output

EXTRA_DIST =			\
	icalparameter.c.in	\
	icalparameter.h.in	\
	icalproperty.c.in	\
	icalproperty.h.in	\
	icalrestriction.c.in	\
	icalvalue.c.in		\
	icalvalue.h.in		\
	$(BUILT_SOURCES)	\
	$(COMBINEDHEADERS)	\
	icallexer.c		\
	icalyacc.c

