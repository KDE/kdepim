###
### The todo conduit is deprecated -- well, it has moved to
### vcalconduit where it shares most of the code with the vcalconduit.
###
### The NULL conduit is a neat programming example, but shouldn't be
### installed on user systems.
###

if include_malconduit
MAL_SUBDIR = malconduit
endif

if include_abc
ABC_SUBDIR = abbrowserconduit vcalconduit
endif

if include_perl
PERL_SUBDIR = perlconduit
endif

if include_python
PYTHON_SUBDIR = pythonconduit
endif

SUBDIRS = \
          docconduit \
          knotes \
          popmail \
          timeconduit \
          sysinfoconduit \
	  notepadconduit \
          $(MAL_SUBDIR) $(ABC_SUBDIR)

###
### Subdirs you might have for experimental purposes:
###
###	null             - a conduit that just logs a single message.
###	$(PERL_SUBDIR)   - fires off a perl interpreter in a thread.
###	$(PYTHON_SUBDIR) - starts a python interpreter in a thread.
###	notepadconduit   - a conduit to save notepad drawings as pngs.
###


