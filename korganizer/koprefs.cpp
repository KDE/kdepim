/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <time.h>
#include <unistd.h>

#include <tqdir.h>
#include <tqstring.h>
#include <tqfont.h>
#include <tqcolor.h>
#include <tqmap.h>
#include <tqstringlist.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kstaticdeleter.h>
#include <kstringhandler.h>

#include <libkmime/kmime_header_parsing.h>

#include "koprefs.h"
#include <libkpimidentities/identitymanager.h>
#include <libkpimidentities/identity.h>
#include <libemailfunctions/email.h>
#include <kabc/stdaddressbook.h>
#include <ktimezones.h>
#include "kocore.h"

KOPrefs *KOPrefs::mInstance = 0;
static KStaticDeleter<KOPrefs> insd;

TQColor getTextColor(const TQColor &c)
{
  float luminance = (c.red() * 0.299) + (c.green() * 0.587) + (c.blue() * 0.114);
  return (luminance > 128.0) ? TQColor( 0, 0 ,0 ) : TQColor( 255, 255 ,255 );
}


KOPrefs::KOPrefs() :
  KOPrefsBase()
{
  mCategoryColors.setAutoDelete( true );
  mResourceColors.setAutoDelete( true );

  mDefaultCategoryColor = TQColor( 151, 235, 121 );

  mDefaultResourceColor = TQColor();//Default is a color invalid

  mDefaultTimeBarFont = KGlobalSettings::generalFont();
  // make a large default time bar font, at least 16 points.
  mDefaultTimeBarFont.setPointSize(
    QMAX( mDefaultTimeBarFont.pointSize() + 4, 16 ) );

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize( mDefaultMonthViewFont.pointSize() - 2 );

  KConfigSkeleton::setCurrentGroup( "General" );

  addItemPath( "Html Export File", mHtmlExportFile,
   TQDir::homeDirPath() + "/" + i18n( "Default export file", "calendar.html" ) );

  timeBarFontItem()->setDefaultValue( mDefaultTimeBarFont );
  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );

  // Load it now, not deep within some painting code
  mMyAddrBookMails = KABC::StdAddressBook::self()->whoAmI().emails();
}


KOPrefs::~KOPrefs()
{
  kdDebug(5850) << "KOPrefs::~KOPrefs()" << endl;
}


KOPrefs *KOPrefs::instance()
{
  if ( !mInstance ) {
    insd.setObject( mInstance, new KOPrefs() );
    mInstance->readConfig();
  }

  return mInstance;
}

void KOPrefs::usrSetDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  KEMailSettings settings;
  TQString tmp = settings.getSetting(KEMailSettings::RealName);
  if ( !tmp.isEmpty() ) setUserName( tmp );
  tmp = settings.getSetting(KEMailSettings::EmailAddress);
  if ( !tmp.isEmpty() ) setUserEmail( tmp );
  fillMailDefaults();

  mTimeBarFont = mDefaultTimeBarFont;
  mMonthViewFont = mDefaultMonthViewFont;

  setTimeZoneIdDefault();

  KPimPrefs::usrSetDefaults();
}

void KOPrefs::fillMailDefaults()
{
  userEmailItem()->swapDefault();
  TQString defEmail = userEmailItem()->value();
  userEmailItem()->swapDefault();

  if ( userEmail() == defEmail ) {
    // No korg settings - but maybe there's a kcontrol[/kmail] setting available
    KEMailSettings settings;
    if ( !settings.getSetting( KEMailSettings::EmailAddress ).isEmpty() )
      mEmailControlCenter = true;
  }
}

void KOPrefs::setTimeZoneIdDefault()
{
  TQString zone;

  zone = KTimezones().local()->name();

  kdDebug() << "----- time zone: " << zone << endl;

  mTimeZoneId = zone;
}

void KOPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();

  mCustomCategories << i18n("Appointment") << i18n("Business")
      << i18n("Meeting") << i18n("Phone Call") << i18n("Education")
      << i18n("Holiday") << i18n("Vacation") << i18n("Special Occasion")
      << i18n("Personal") << i18n("Travel") << i18n("Miscellaneous")
      << i18n("Birthday");
}


void KOPrefs::usrReadConfig()
{
  config()->setGroup("General");
  mCustomCategories = config()->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  // old category colors, ignore if they have the old default
  // should be removed a few versions after 3.2...
  config()->setGroup("Category Colors");
  TQValueList<TQColor> oldCategoryColors;
  TQStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    TQColor c = config()->readColorEntry(*it, &mDefaultCategoryColor);
    oldCategoryColors.append( (c == TQColor(196,196,196)) ?
                              mDefaultCategoryColor : c);
  }

  // new category colors
  config()->setGroup("Category Colors2");
  TQValueList<TQColor>::Iterator it2;
  for (it = mCustomCategories.begin(), it2 = oldCategoryColors.begin();
       it != mCustomCategories.end(); ++it, ++it2 ) {
      TQColor c = config()->readColorEntry(*it, &*it2);
      if ( c != mDefaultCategoryColor )
          setCategoryColor(*it,c);
  }

  config()->setGroup( "Resources Colors" );
  TQMap<TQString, TQString> map = config()->entryMap( "Resources Colors" );

  TQMapIterator<TQString, TQString> it3;
  for( it3 = map.begin(); it3 != map.end(); ++it3 ) {
    // kdDebug(5850)<< "KOPrefs::usrReadConfig: key: " << it3.key() << " value: "
    //  << it3.data()<<endl;
    setResourceColor( it3.key(), config()->readColorEntry( it3.key(),
      &mDefaultResourceColor ) );
  }


  if (mTimeZoneId.isEmpty()) {
    setTimeZoneIdDefault();
  }

#if 0
  config()->setGroup("FreeBusy");
  if( mRememberRetrievePw )
    mRetrievePassword = KStringHandler::obscure( config()->readEntry( "Retrieve Server Password" ) );
#endif
  KPimPrefs::usrReadConfig();
  fillMailDefaults();
}


void KOPrefs::usrWriteConfig()
{
  config()->setGroup("General");
  config()->writeEntry("Custom Categories",mCustomCategories);

  config()->setGroup("Category Colors2");
  TQDictIterator<TQColor> it(mCategoryColors);
  while (it.current()) {
    config()->writeEntry(it.currentKey(),*(it.current()));
    ++it;
  }

  config()->setGroup( "Resources Colors" );
  TQDictIterator<TQColor> it2( mResourceColors );
  while( it2.current() ) {
    config()->writeEntry( it2.currentKey(), *( it2.current() ) );
    ++it2;
  }

  if( !mFreeBusyPublishSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyPublishPasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }
  if( !mFreeBusyRetrieveSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyRetrievePasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }

#if 0
  if( mRememberRetrievePw )
    config()->writeEntry( "Retrieve Server Password", KStringHandler::obscure( mRetrievePassword ) );
  else
    config()->deleteEntry( "Retrieve Server Password" );
#endif

  KPimPrefs::usrWriteConfig();
}

void KOPrefs::setCategoryColor( const TQString &cat, const TQColor & color)
{
  mCategoryColors.replace( cat, new TQColor( color ) );
}

TQColor *KOPrefs::categoryColor( const TQString &cat )
{
  TQColor *color = 0;

  if ( !cat.isEmpty() ) color = mCategoryColors[ cat ];

  if ( color ) return color;
  else return &mDefaultCategoryColor;
}


bool KOPrefs::hasCategoryColor( const TQString& cat ) const
{
    return mCategoryColors[ cat ];
}

void KOPrefs::setResourceColor ( const TQString &cal, const TQColor &color )
{
  // kdDebug(5850)<<"KOPrefs::setResourceColor: " << cal << " color: "<<
  // color.name()<<endl;
  mResourceColors.replace( cal, new TQColor( color ) );
}

TQColor* KOPrefs::resourceColor( const TQString &cal )
{
  TQColor *color=0;
  if( !cal.isEmpty() ) color = mResourceColors[cal];

  // assign default color if enabled
  if ( !cal.isEmpty() && !color && assignDefaultResourceColors() ) {
    TQColor defColor( 0x37, 0x7A, 0xBC );
    if ( defaultResourceColorSeed() > 0 && defaultResourceColorSeed() - 1 < (int)defaultResourceColors().size() ) {
        defColor = TQColor( defaultResourceColors()[defaultResourceColorSeed()-1] );
    } else {
        int h, s, v;
        defColor.getHsv( h, s, v );
        h = ( defaultResourceColorSeed() % 12 ) * 30;
        s -= s * ( (defaultResourceColorSeed() / 12) % 2 ) * 0.5;
        defColor.setHsv( h, s, v );
    }
    setDefaultResourceColorSeed( defaultResourceColorSeed() + 1 );
    setResourceColor( cal, defColor );
    color = mResourceColors[cal];
  }

  if (color && color->isValid() )
    return color;
  else
    return &mDefaultResourceColor;
}

TQString KOPrefs::fullName()
{
  TQString tusername;
  if ( mEmailControlCenter ) {
    KEMailSettings settings;
    tusername = settings.getSetting( KEMailSettings::RealName );
  } else {
    tusername = userName();
  }

  // Quote the username as it might contain commas and other quotable chars.
  tusername = KPIM::quoteNameIfNecessary( tusername );

  TQString tname, temail;
  KPIM::getNameAndMail( tusername, tname, temail ); // ignore return value
                                                    // which is always false
  return tname;
}

TQString KOPrefs::email()
{
  if ( mEmailControlCenter ) {
    KEMailSettings settings;
    return settings.getSetting( KEMailSettings::EmailAddress );
  } else {
    return userEmail();
  }
}

TQStringList KOPrefs::allEmails()
{
  // Grab emails from the email identities
  TQStringList lst = KOCore::self()->identityManager()->allEmails();
  // Add emails configured in korganizer
  lst += mAdditionalMails;
  // Add emails from the user's kaddressbook entry
  lst += mMyAddrBookMails;
  // Add the email entered as the userEmail here
  lst += email();

  // Warning, this list could contain duplicates.
  return lst;
}

TQStringList KOPrefs::fullEmails()
{
  TQStringList fullEmails;
  // The user name and email from the config dialog:
  fullEmails << TQString("%1 <%2>").arg( fullName() ).arg( email() );

  TQStringList::Iterator it;
  // Grab emails from the email identities
  KPIM::IdentityManager *idmanager = KOCore::self()->identityManager();
  TQStringList lst = idmanager->identities();
  KPIM::IdentityManager::ConstIterator it1;
  for ( it1 = idmanager->begin() ; it1 != idmanager->end() ; ++it1 ) {
    fullEmails << (*it1).fullEmailAddr();
  }
  // Add emails configured in korganizer
  lst = mAdditionalMails;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << TQString("%1 <%2>").arg( fullName() ).arg( *it );
  }
  // Add emails from the user's kaddressbook entry
  KABC::Addressee me = KABC::StdAddressBook::self()->whoAmI();
  lst = me.emails();
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << me.fullEmail( *it );
  }

  // Warning, this list could contain duplicates.
  return fullEmails;
}

bool KOPrefs::thatIsMe( const TQString& _email )
{
  // NOTE: this method is called for every created agenda view item, so we need to keep
  // performance in mind

  /* identityManager()->thatIsMe() is quite expensive since it does parsing of
     _email in a way which is unnecessarily complex for what we can have here,
     so we do that ourselves. This makes sense since this

  if ( KOCore::self()->identityManager()->thatIsMe( _email ) )
    return true;
  */

  // in case email contains a full name, strip it out
  // the below is the simpler but slower version of the following KMime code
  // const TQString email = KPIM::getEmailAddress( _email );
  const TQCString tmp = _email.utf8();
  const char *cursor = tmp.data();
  const char *end = tmp.data() + tmp.length();
  KMime::Types::Mailbox mbox;
  KMime::HeaderParsing::parseMailbox( cursor, end, mbox );
  const TQString email = mbox.addrSpec.asString();

  for ( KPIM::IdentityManager::ConstIterator it = KOCore::self()->identityManager()->begin();
        it != KOCore::self()->identityManager()->end(); ++it ) {
    if ( email == (*it).primaryEmailAddress() )
      return true;
    const TQStringList & aliases = (*it).emailAliases();
    if ( aliases.find( email ) != aliases.end() )
        return true;
  }

  if ( mAdditionalMails.find( email ) != mAdditionalMails.end() )
    return true;
  TQStringList lst = mMyAddrBookMails;
  if ( lst.find( email ) != lst.end() )
    return true;
  return false;
}
