add_subdirectory(newsgatorresource)
add_subdirectory(opmlresource)
add_subdirectory(feederagent)

# simple check for the Akonadi filtering framework
# from playground/pim/akonadi/filter
IF( EXISTS ${KDE4_INCLUDE_DIR}/akonadi/filter/data.h )
  add_subdirectory(filteringagent)
ENDIF( EXISTS ${KDE4_INCLUDE_DIR}/akonadi/filter/data.h )
