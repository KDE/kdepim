# This file contains all the specific settings that will be used
# when running 'make Experimental'

# Change the maximum warnings that will be displayed
# on the report page (default 50)
set(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 2000)

# Warnings that will be ignored
set(CTEST_CUSTOM_WARNING_EXCEPTION
  
  "groupwise/soap"
  "mk4storage"
  "too big, try a different debug format"
  )

# Errors that will be ignored
set(CTEST_CUSTOM_ERROR_EXCEPTION
  
  "ICECC"
  "Segmentation fault"
  )

# No coverage for these files
set(CTEST_CUSTOM_COVERAGE_EXCLUDE ".moc$" "moc_" "ui_" "ontologies")
