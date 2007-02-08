include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckPrototypeExists)
include(CheckTypeSize)
include(MacroBoolTo01)

# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if (WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES} )
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES} )
endif (WIN32)

OPTION(KDE4_KDEPIM_NEW_DISTRLISTS   "Whether to use new distribution lists, to store them like normal contacts; useful for Kolab") 

macro_bool_to_01(KDE4_KDEPIM_NEW_DISTRLISTS KDEPIM_NEW_DISTRLISTS)
macro_bool_to_01(SASL2_FOUND HAVE_LIBSASL2)
macro_bool_to_01(GNOKII_FOUND HAVE_GNOKII_H)

OPTION(WITH_INDEXLIB "Enable full-text indexing in KMail." OFF)
macro_bool_to_01(WITH_INDEXLIB HAVE_INDEXLIB)

check_type_size("long" SIZEOF_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
