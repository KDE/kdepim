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

SUBDIRS = \
          docconduit \
          knotes \
          timeconduit \
          sysinfoconduit \
          $(MAL_SUBDIR) $(ABC_SUBDIR)
