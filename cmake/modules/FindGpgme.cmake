# - Try to find the gpgme library
# Once done this will define
#
#  GPGME_FOUND - system has the gpgme library
#  GPGME_INCLUDES - the gpgme include directory
#  GPGME_LIBRARIES - The libraries needed to use gpgme

include (MacroEnsureVersion)

# if not already in cache
if (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
  set(GPGME_FOUND FALSE)

  FIND_PROGRAM(GPGMECONFIG_EXECUTABLE NAMES gpgme-config)

  # if gpgme-config has been found
  if (GPGMECONFIG_EXECUTABLE)

	message(STATUS "Found gpgme-config")

    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE GPGME_VERSION)

    MACRO_ENSURE_VERSION( "0.4.5" ${GPGME_VERSION} GPGME_INSTALLED_VERSION_OK )

    if (GPGME_INSTALLED_VERSION_OK)

      EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --libs OUTPUT_VARIABLE GPGME_LIBRARIES)

      # append -lgpg-error to the list of libraries, if necessary
      if (NOT GPGME_LIBRARIES MATCHES "lgpg-error")
        set(GPGME_LIBRARIES "${GPGME_LIBRARIES} -lgpg-error")
      endif (NOT GPGME_LIBRARIES MATCHES "lgpg-error")

      EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE GPGME_CFLAGS)
      if (GPGME_CFLAGS)
        string(REGEX REPLACE "(\r?\n)+$" "" GPGME_CFLAGS "${GPGME_CFLAGS}")
        string(REGEX REPLACE " *-I" ";" GPGME_INCLUDES "${GPGME_CFLAGS}")
      endif (GPGME_CFLAGS)

      # ensure that they are cached
      set(GPGME_INCLUDES ${GPGME_INCLUDES} CACHE INTERNAL "The gpgme include paths")
      set(GPGME_LIBRARIES ${GPGME_LIBRARIES} CACHE INTERNAL "The gpgme libraries")

    else (GPGME_INSTALLED_VERSION_OK)
      message(STATUS "The installed version of gpgme is too old: ${GPGME_VERSION}")
    endif (GPGME_INSTALLED_VERSION_OK)

  endif (GPGMECONFIG_EXECUTABLE)

  if (GPGME_LIBRARIES)
      set(GPGME_FOUND TRUE)
  endif (GPGME_LIBRARIES)

else (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
  # It was found before, assume it's OK
  set(GPGME_FOUND TRUE)
endif (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)

macro_bool_to_01(GPGME_FOUND HAVE_GPGME)

if (NOT Gpgme_FIND_QUIETLY)
   if (GPGME_FOUND)
      message(STATUS "Found gpgme.")
   else (GPGME_FOUND)
      message(STATUS "gpg not found.")
   endif (GPGME_FOUND)
   if (Gpgme_FIND_REQUIRED)
      set (_req TRUE)
   else (Gpgme_FIND_REQUIRED)
      set (_req FALSE)
   endif (Gpgme_FIND_REQUIRED)
   macro_log_feature(GPGME_FOUND "gpgme" "GnuPG Made Easy Development Libraries" "http://www.gnupg.org/related_software/gpgme" ${_req} "0.4.5 or greater" "Needed to provide GNU Privacy Guard support in KDE PIM applications. Necessary to compile many PIM application, including KMail.")
else (NOT Gpgme_FIND_QUIETLY)
   if (NOT GPGME_FOUND)
      if (Gpgme_FIND_REQUIRED)
         message(FATAL_ERROR "")
      endif (Gpgme_FIND_REQUIRED)
   endif (NOT GPGME_FOUND)
endif (NOT Gpgme_FIND_QUIETLY)


