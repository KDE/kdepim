add_subdirectory(newsgatorresource)
add_subdirectory(opmlresource)

IF( Nepomuk_FOUND AND SOPRANO_PLUGIN_RAPTORPARSER_FOUND )
  add_subdirectory(feederagent)
ENDIF( Nepomuk_FOUND AND SOPRANO_PLUGIN_RAPTORPARSER_FOUND )

# simple check for the Akonadi filtering framework
# from playground/pim/akonadi/filter
IF( EXISTS ${KDE4_INCLUDE_DIR}/akonadi/filter/data.h )
  add_subdirectory(filteringagent)
ENDIF( EXISTS ${KDE4_INCLUDE_DIR}/akonadi/filter/data.h )
