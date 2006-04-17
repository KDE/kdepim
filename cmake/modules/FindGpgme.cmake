# - Try to find the gpgme library
# Once done this will define
#
#  GPGME_FOUND - system has the gpgme library
#  GPGME_INCLUDES - the gpgme include directory
#  GPGME_LIBRARIES - The libraries needed to use gpgme

# if not already in cache
IF (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
  set(GPGME_FOUND FALSE)

  FIND_PROGRAM(GPGMECONFIG_EXECUTABLE NAMES gpgme-config)

  # if gpgme-config has been found
  IF (GPGMECONFIG_EXECUTABLE)
    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_LIBRARIES)

    # append -lgpg-error to the list of libraries, if necessary
    IF (NOT GPGME_LIBRARIES MATCHES "lgpg-error")
      set(GPGME_LIBRARIES "${GPGME_LIBRARIES} -lgpg-error")
    ENDIF (NOT GPGME_LIBRARIES MATCHES "lgpg-error")

    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_CFLAGS)
    IF (GPGME_CFLAGS)
      # problem: here I get two identical -I directives, and this ends up as one path with a space in the var...
      # so for now I just keep the first -Ifoo, but this might need to be fixed at some point...
      STRING(REGEX REPLACE "-I([^ ]*).*" "\\1" GPGME_INCLUDES "${GPGME_CFLAGS}")
      # next problem: I get no -I directive here at all, and this ends up with an empty -I in the compiler call
      STRING(REGEX REPLACE "\n" " " GPGME_INCLUDES "${GPGME_INCLUDES}")
    ENDIF (GPGME_CFLAGS)

    # ensure that they are cached
    set(GPGME_INCLUDES ${GPGME_INCLUDES} CACHE INTERNAL "The gpgme include paths")
    set(GPGME_LIBRARIES ${GPGME_LIBRARIES} CACHE INTERNAL "The gpgme libraries")

  ENDIF (GPGMECONFIG_EXECUTABLE)

  IF (GPGME_LIBRARIES AND GPGME_INCLUDES)
      set(GPGME_FOUND TRUE)
      #message(STATUS "Found gpgme: includes: ${GPGME_INCLUDES} libs: ${GPGME_LIBRARIES}")
      message(STATUS "Found gpgme using ${GPGMECONFIG_EXECUTABLE}.")
  ENDIF (GPGME_LIBRARIES AND GPGME_INCLUDES)

  IF (NOT GPGME_FOUND)
      message(FATAL_ERROR "You are missing gpgme 0.4.5 or higher.
    Download gpgme >= 0.4.5 from ftp://ftp.gnupg.org/gcrypt/alpha/gpgme
    or use the --with-gpgme-prefix=/path/where/gpgme/is/installed option.")
  ENDIF (NOT GPGME_FOUND)
ENDIF (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
