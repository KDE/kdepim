# Modified to not be completely useless by Marc Mutz <mutz@kde.org>
# Changes made:
# 1. s/Subversion/KdeSubversion/
# 2. Remove LAST_CHANGED_LOG, which requires network access.

# - Extract information from a subversion working copy
# The module defines the following variables:
#  KdeSubversion_SVN_EXECUTABLE - path to svn command line client
#  KdeSubversion_VERSION_SVN - version of svn command line client
#  KdeSubversion_FOUND - true if the command line client was found
# If the command line client executable is found the macro
#  KdeSubversion_WC_INFO(<dir> <var-prefix>)
# is defined to extract information of a subversion working copy at
# a given location. The macro defines the following variables:
#  <var-prefix>_WC_URL - url of the repository (at <dir>)
#  <var-prefix>_WC_ROOT - root url of the repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_LAST_CHANGED_LOG - last log of base revision
#  <var-prefix>_WC_INFO - output of command `svn info <dir>'
# Example usage:
#  FIND_PACKAGE(KdeSubversion)
#  IF(KdeSubversion_FOUND)
#    KdeSubversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#  ENDIF(KdeSubversion_FOUND)

# Copyright (c) 2006, Tristan Carel
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the University of California, Berkeley nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# $Id: FindSubversion.cmake,v 1.1.2.1 2006/11/13 17:59:54 hoffman Exp $

SET(KdeSubversion_FOUND FALSE)
SET(KdeSubversion_SVN_FOUND FALSE)

FIND_PROGRAM(KdeSubversion_SVN_EXECUTABLE svn
  DOC "subversion command line client")
MARK_AS_ADVANCED(KdeSubversion_SVN_EXECUTABLE)

IF(KdeSubversion_SVN_EXECUTABLE)
  SET(KdeSubversion_SVN_FOUND TRUE)
  SET(KdeSubversion_FOUND TRUE)

  MACRO(KdeSubversion_WC_INFO dir prefix)
    EXECUTE_PROCESS(COMMAND ${KdeSubversion_SVN_EXECUTABLE} --version
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      OUTPUT_VARIABLE KdeSubversion_VERSION_SVN
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    EXECUTE_PROCESS(COMMAND ${KdeSubversion_SVN_EXECUTABLE} info ${dir}
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE KdeSubversion_svn_info_error
      RESULT_VARIABLE KdeSubversion_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${KdeSubversion_svn_info_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${KdeSubversion_SVN_EXECUTABLE} info ${dir}\" failed with output:\n${KdeSubversion_svn_info_error}")
    ELSE(NOT ${KdeSubversion_svn_info_result} EQUAL 0)

      STRING(REGEX REPLACE "^(.*\n)?svn, version ([.0-9]+).*"
        "\\2" KdeSubversion_VERSION_SVN "${KdeSubversion_VERSION_SVN}")
      STRING(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"
        "\\2" ${prefix}_WC_URL "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
        "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_REV "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_DATE "${${prefix}_WC_INFO}")

    ENDIF(NOT ${KdeSubversion_svn_info_result} EQUAL 0)

  ENDMACRO(KdeSubversion_WC_INFO)

ENDIF(KdeSubversion_SVN_EXECUTABLE)

IF(NOT KdeSubversion_FOUND)
  IF(NOT KdeSubversion_FIND_QUIETLY)
    MESSAGE(STATUS "Subversion was not found.")
  ELSE(NOT KdeSubversion_FIND_QUIETLY)
    IF(KdeSubversion_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Subversion was not found.")
    ENDIF(KdeSubversion_FIND_REQUIRED)
  ENDIF(NOT KdeSubversion_FIND_QUIETLY)
ENDIF(NOT KdeSubversion_FOUND)

# FindKdeSubversion.cmake ends here.
