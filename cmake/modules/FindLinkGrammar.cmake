# Try to find Linkgrammar
# Once done, this module with define:
# LinkGrammar_FOUND         - True if LinkGrammar was found
# LinkGrammar_INCLUDE_DIR   - The LinkGrammar include directory
# LinkGrammar_LIBRARIES     - The LinkGrammar libraries

# Copyright (c) 2013, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(LinkGrammar_INCLUDE_DIR link-grammar/link-includes.h)

find_library(LinkGrammar_LIBRARIES NAMES link-grammar)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LinkGrammar DEFAULT_MSG LinkGrammar_LIBRARIES LinkGrammar_INCLUDE_DIR)

mark_as_advanced(LinkGrammar_INCLUDE_DIR LinkGrammar_LIBRARIES)
