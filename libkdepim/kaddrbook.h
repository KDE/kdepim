/* Simple Addressbook for KMail
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef KAddrBook_h
#define KAddrBook_h

#include <tqstringlist.h>

#include <kdeversion.h>
#include <kabc/addressee.h>
#include <kdepimmacros.h>

namespace KABC {
  class AddressBook;
}

class TQWidget;

class KDE_EXPORT KAddrBookExternal {
public:
  static void addEmail( const TQString &addr, TQWidget *parent );
  static void addNewAddressee( TQWidget* );
  static void openEmail( const TQString &addr, TQWidget *parent );
  static void openAddressBook( TQWidget *parent );

  static bool addVCard( const KABC::Addressee& addressee, TQWidget *parent );

  static TQString expandDistributionList( const TQString& listName );

  /**
   * Pops up a dialog to ask the user to select a resource for saving something, and
   * returns the selected resource or 0 on failure or if the user cancelled.
   *
   * The addressbook used to get the resource list from. If the addressbook was loaded
   * async and loading is not yet finished, this method will run an eventloop until the
   * addressbook is loaded.
   */
  static KABC::Resource* selectResourceForSaving( KABC::AddressBook *addressBook );

private:
  static bool addAddressee( const KABC::Addressee& addressee );
};

#endif /*KAddrBook_h*/
