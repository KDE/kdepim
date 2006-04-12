# - Try to find the gpgme library
# Once done this will define
#
#  GPGME_FOUND - system has the gpgme library
#  GPGME_INCLUDE_DIR - the gpgme include directory
#  GPGME_LIBRARIES - The libraries needed to use gpgme

FIND_PROGRAM(GPGMECONFIG_EXECUTABLE NAMES gpgme-config)

#reset vars
set(GPGME_LIBRARIES)
set(GPGME_INCLUDE_DIR)

# if gpgme-config has been found
IF (GPGMECONFIG_EXECUTABLE)

  EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_LIBRARIES)

  EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_INCLUDE_DIR)

  IF (GPGME_LIBRARIES AND GPGME_INCLUDE_DIR)
    SET(GPGME_FOUND TRUE)
    STRING(REGEX REPLACE "-I(.+)" "\\1" GPGME_INCLUDE_DIR "${GPGME_INCLUDE_DIR}")
    message(STATUS "Found gpgme: ${GPGME_LIBRARIES}")
  ENDIF (GPGME_LIBRARIES AND GPGME_INCLUDE_DIR)

  IF (NOT GPGME_FOUND)
    message(FATAL_ERROR "You are missing gpgme 0.4.5 or higher.
  Download gpgme >= 0.4.5 from ftp://ftp.gnupg.org/gcrypt/alpha/gpgme
  or use the --with-gpgme-prefix=/path/where/gpgme/is/installed option.")

  ENDIF (NOT GPGME_FOUND)

  MARK_AS_ADVANCED(GPGME_INCLUDE_DIR GPGME_LIBRARIES)

ENDIF (GPGMECONFIG_EXECUTABLE)
