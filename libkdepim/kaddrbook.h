/* Simple Addressbook for KMail
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef KAddrBook_h
#define KAddrBook_h

#include <qstringlist.h>

#include <kdeversion.h>
#include <kabc/addressee.h>
#include <kdepimmacros.h>

namespace KABC {
  class AddressBook;
}

class QWidget;

class KDE_EXPORT KAddrBookExternal {
public:
  static void addEmail( const QString &addr, QWidget *parent );
  static void addNewAddressee( QWidget* );
  static void openEmail( const QString &addr, QWidget *parent );
  static void openAddressBook( QWidget *parent );

  static bool addVCard( const KABC::Addressee& addressee, QWidget *parent );

  static QString expandDistributionList( const QString& listName );

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
