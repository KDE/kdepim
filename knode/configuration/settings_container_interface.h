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

#ifndef KNODE_SETTINGS_CONTAINER_INTERFACE_H
#define KNODE_SETTINGS_CONTAINER_INTERFACE_H

namespace KPIMIdentities {
  class Identity;
}

namespace KNode {

/**
  Interface for object (global settings, account and group) that contains common
  settings (like identity or cleanup).

  This will ease loading and saving settings inside configuration widgets.

  This does not
*/
class SettingsContainerInterface
{
  public:
    virtual ~SettingsContainerInterface() {}

    /**
      Returns the identity configured for this container.

      It is the null identity if there is none.
    */
    virtual const KPIMIdentities::Identity & identity() const = 0;
    /**
      Sets the identity for this group.
      @param identity The identity or a null Identity to unset it.
    */
    virtual void setIdentity( const KPIMIdentities::Identity &identity ) = 0;

    /**
      Save the configuration to disk
    */
    virtual void writeConfig() = 0;

  protected:
    explicit SettingsContainerInterface() {}
};

}

#endif //KNODE_SETTINGS_CONTAINER_INTERFACE_H
