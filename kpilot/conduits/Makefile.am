###
### The NULL conduit is a neat programming example, but shouldn't be
### installed on user systems.
###

if include_malconduit
MAL_SUBDIR = malconduit
else
MAL_SUBDIR = 
endif

SUBDIRS = \
	  abbrowserconduit \
	  docconduit \
	  knotes \
	  $(MAL_SUBDIR) \
	  memofileconduit \
	  notepadconduit \
	  popmail \
	  sysinfoconduit \
	  timeconduit \
	  vcalconduit

###
### Subdirs you might have for experimental purposes:
###
###	null             - a conduit that just logs a single message.
###	$(PERL_SUBDIR)   - fires off a perl interpreter in a thread.
###	$(PYTHON_SUBDIR) - starts a python interpreter in a thread.
###
