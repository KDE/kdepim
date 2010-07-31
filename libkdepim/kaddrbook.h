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

class QWidget;

class KDE_EXPORT KAddrBookExternal {
public:
  static void addEmail( const TQString &addr, TQWidget *parent );
  static void addNewAddressee( TQWidget* );
  static void openEmail( const TQString &addr, TQWidget *parent );
  static void openAddressBook( TQWidget *parent );

  static bool addVCard( const KABC::Addressee& addressee, TQWidget *parent );

  static TQString expandDistributionList( const TQString& listName );
private:
  static bool addAddressee( const KABC::Addressee& addressee );
};

#endif /*KAddrBook_h*/
