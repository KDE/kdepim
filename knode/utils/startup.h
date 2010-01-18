/*
 Copyright 2009 Olivier Trichet <nive@nivalis.org>

 Permission to use, copy, modify, and distribute this software
 and its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appear in all
 copies and that both that the copyright notice and this
 permission notice and warranty disclaimer appear in supporting
 documentation, and that the name of the author not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.

 The author disclaim all warranties with regard to this
 software, including all implied warranties of merchantability
 and fitness.  In no event shall the author be liable for any
 special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether
 in an action of contract, negligence or other tortious action,
 arising out of or in connection with the use or performance of
 this software.
*/

#ifndef KNODE_UTILITIES_STARTUP_H
#define KNODE_UTILITIES_STARTUP_H

#include "knode_export.h"

#include <KConfigGroup>


namespace KNode {
namespace Utilities {

/**
  @brief A class to deals with start-up/initialization of KNode.
*/
class KNODE_EXPORT Startup
{
  public:
    /**
      Loads translation catalogs and icons directories for imported libraries.
    */
    void loadLibrariesIconsAndTranslations() const;

    /**
      Updates internal data at startup.
      Whenever possible, use kconf_update instead.
    */
    void updateDataAndConfiguration() const;

  private:
    /**
      Convert KNode-specific Identity objects stored in configuration of KNode and accounts and groups
      to KPIMIdentities::Identity.
      @since 4.5
    */
    void convertPre45Identities() const;
    /**
      Convert a KNode-specific identity from a group of a (global, account or group) configuration as
      stored in KNode up to version 4.4.x (included).
    */
    void convertPre45Identity( KConfigGroup &cg ) const;
};


} // namespace Utilities
} // namespace KNode

#endif // KNODE_UTILITIES_STARTUP_H
