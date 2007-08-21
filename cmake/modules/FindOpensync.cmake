# Find OpenSync
#
#  OPENSYNC_FOUND - system has OpenSync
#  OPENSYNC_INCLUDE_DIRS - the OpenSync include directory
#  OPENSYNC_LDFLAGS - The libraries needed to use OpenSync
#  GLIB_FOUND - system has Glib
#  GLIB_INCLUDE_DIRS - the glib include directory
#  GLIB_LDFLAGS - The libraries need to use Glib
#

INCLUDE(FindPkgConfig)

if (OPENSYNC_INCLUDEDIR AND OPENSYNC_LIBDIR)

  # in cache already
  SET(OPENSYNC_FOUND TRUE)

else (OPENSYNC_INCLUDEDIR AND OPENSYNC_LIBDIR)

  SET(OPENSYNC_FOUND FALSE)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    PKG_CHECK_MODULES(OPENSYNC opensync-1.0>=0.30)
  ENDIF(NOT WIN32)

endif (OPENSYNC_INCLUDEDIR AND OPENSYNC_LIBDIR)

if (NOT Opensync_FIND_QUIETLY)
   if (Opensync_FIND_REQUIRED)
      set (_req TRUE)
   else (Opensync_FIND_REQUIRED)
      set (_req FALSE)
   endif (Opensync_FIND_REQUIRED)
   macro_log_feature(OPENSYNC_FOUND "opensync" "OpenSync Development Libraries" "http://www.opensync.org" ${_req} "0.19 or greater" "Needed to provide synching applications from KDE PIM applications. Necessary to compile kitchensync and other PIM applications.")
else (NOT Opensync_FIND_QUIETLY)
   if (NOT OPENSYNC_FOUND)
      if (Opensync_FIND_REQUIRED)
         message(FATAL_ERROR "")
      endif (Opensync_FIND_REQUIRED)
   endif (NOT OPENSYNC_FOUND)
endif (NOT Opensync_FIND_QUIETLY)



if (GLIB_INCLUDEDIR AND GLIB_LIBDIR)

  # in cache already
  SET(GLIB_FOUND TRUE)

else (GLIB_INCLUDEDIR AND GLIB_LIBDIR)

  SET(GLIB_FOUND FALSE)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    PKG_CHECK_MODULES(GLIB glib-2.0)
  ENDIF(NOT WIN32)

endif (GLIB_INCLUDEDIR AND GLIB_LIBDIR)

if (NOT Opensync_FIND_QUIETLY)
   if (Opensync_FIND_REQUIRED)
      set (_req TRUE)
   else (Opensync_FIND_REQUIRED)
      set (_req FALSE)
   endif (Opensync_FIND_REQUIRED)
   macro_log_feature(GLIB_FOUND "glib" "Glib Development Libraries" ${_req} "Needed to provide synching applications from KDE PIM applications. Necessary to compile kitchensync and other PIM applications.")
else (NOT Opensync_FIND_QUIETLY)
   if (NOT GLIB_FOUND)
      if (Opensync_FIND_REQUIRED)
         message(FATAL_ERROR "")
      endif (Opensync_FIND_REQUIRED)
   endif (NOT GLIB_FOUND)
endif (NOT Opensync_FIND_QUIETLY)
