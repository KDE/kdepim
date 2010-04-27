/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Kevin Krammer, krake@kdab.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "prefs.h"

#include "prefs_base.h"

//TODO_SPLIT
//#include "kocore.h"

//#include "categoryconfig.h"

#include <akonadi/collection.h>

#include <kmime/kmime_header_parsing.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <kpimutils/email.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kdatetime.h>
#include <kstringhandler.h>
#include <ksystemtimezone.h>

#include <QDir>
#include <QHash>
#include <QString>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QStringList>

#include <time.h>
#include <unistd.h>

using namespace KPIMIdentities;

using namespace EventViews;

Prefs *Prefs::mInstance = 0;

class Prefs::Private : public PrefsBase
{
  public:
    Private( Prefs *parent ) : PrefsBase(), q( parent ) {}

    void init();

    void usrSetDefaults();
    void usrReadConfig();
    void usrWriteConfig();

    void fillMailDefaults();
    void setTimeZoneDefault();

  public:
    QString mHtmlExportFile;

    // Groupware passwords
    QString mPublishPassword;
    QString mRetrievePassword;

    QHash<QString,QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QHash<QString,QColor> mResourceColors;
    QColor mDefaultResourceColor;

    QFont mDefaultMonthViewFont;
    QFont mDefaultAgendaTimeLabelsFont;

    KDateTime::Spec mTimeSpec;
    QStringList mTimeScaleTimeZones;

    QString mDefaultCalendar;
    Akonadi::Collection mDefaultCollection;

  private:
    Prefs *q;
};

void Prefs::Private::init()
{
  mDefaultCategoryColor = QColor( 151, 235, 121 );
  mDefaultResourceColor = QColor(); //Default is a color invalid

  mDefaultAgendaTimeLabelsFont = KGlobalSettings::generalFont();
  // make a large default time bar font, at least 16 points.
  mDefaultAgendaTimeLabelsFont.setPointSize(
    qMax( mDefaultAgendaTimeLabelsFont.pointSize() + 4, 16 ) );

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize(
    qMax( mDefaultMonthViewFont.pointSize() - 2, 6 ) );

  KConfigSkeleton::setCurrentGroup( "General" );

  addItemPath( "Html Export File", mHtmlExportFile,
      QDir::homePath() + '/' + i18nc( "Default export file", "calendar.html" ) );

  agendaTimeLabelsFontItem()->setDefaultValue( mDefaultAgendaTimeLabelsFont );
  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );
}

void Prefs::Private::usrSetDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  KEMailSettings settings;
  QString tmp = settings.getSetting( KEMailSettings::RealName );
  if ( !tmp.isEmpty() ) {
    setUserName( tmp );
  }
  tmp = settings.getSetting( KEMailSettings::EmailAddress );
  if ( !tmp.isEmpty() ) {
    setUserEmail( tmp );
  }

  fillMailDefaults();

  setAgendaTimeLabelsFont( mDefaultAgendaTimeLabelsFont );
  setMonthViewFont( mDefaultMonthViewFont );

  setTimeZoneDefault();

  KConfigSkeleton::usrSetDefaults();
}

void Prefs::Private::usrReadConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  // Note that the [Category Colors] group was removed after 3.2 due to
  // an algorithm change. That's why we now use [Category Colors2]

  // Category colors
  KConfigGroup colorsConfig( config(), "Category Colors2" );
/*  CategoryConfig cc( this );
  const QStringList cats = cc.customCategories();
  Q_FOREACH( const QString& i, cats ) {
    QColor c = colorsConfig.readEntry( i, mDefaultCategoryColor );
    if ( c != mDefaultCategoryColor ) {
      setCategoryColor( i, c );
    }
    }*/

  // Resource colors
  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  const QStringList colorKeyList = rColorsConfig.keyList();

  QStringList::ConstIterator it3;
  for ( it3 = colorKeyList.begin(); it3 != colorKeyList.end(); ++it3 ) {
    QColor color = rColorsConfig.readEntry( *it3, mDefaultResourceColor );
    //kDebug() << "key:" << (*it3) << "value:" << color;
    q->setResourceColor( *it3, color );
  }

  if ( !mTimeSpec.isValid() ) {
    setTimeZoneDefault();
  }

#if 0
  config()->setGroup( "FreeBusy" );
  if ( mRememberRetrievePw ) {
    mRetrievePassword =
      KStringHandler::obscure( config()->readEntry( "Retrieve Server Password" ) );
  }
#endif
  KConfigGroup defaultCalendarConfig( config(), "Calendar" );
  mDefaultCalendar = defaultCalendarConfig.readEntry( "Default Calendar", QString() );

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  q->setTimeScaleTimezones( timeScaleConfig.readEntry( "Timescale Timezones", QStringList() ) );

  KConfigSkeleton::usrReadConfig();
  fillMailDefaults();
}

void Prefs::Private::usrWriteConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  KConfigGroup colorsConfig( config(), "Category Colors2" );
  QHash<QString, QColor>::const_iterator i = mCategoryColors.constBegin();
  while ( i != mCategoryColors.constEnd() ) {
    colorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  i = mResourceColors.constBegin();
  while ( i != mResourceColors.constEnd() ) {
    rColorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

  if ( !mFreeBusyPublishSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyPublishPasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }
  if ( !mFreeBusyRetrieveSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyRetrievePasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }

#if 0
  if ( mRememberRetrievePw ) {
    config()->writeEntry( "Retrieve Server Password",
                          KStringHandler::obscure( mRetrievePassword ) );
  } else {
    config()->deleteEntry( "Retrieve Server Password" );
  }
#endif

  KConfigGroup defaultCalendarConfig( config(), "Calendar" );
  defaultCalendarConfig.writeEntry( "Default Calendar", q->defaultCalendar() );

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  timeScaleConfig.writeEntry( "Timescale Timezones", q->timeScaleTimezones() );


  KConfigSkeleton::usrWriteConfig();
}

void Prefs::Private::fillMailDefaults()
{
  userEmailItem()->swapDefault();
  QString defEmail = userEmailItem()->value();
  userEmailItem()->swapDefault();

  if ( userEmail() == defEmail ) {
    // No korg settings - but maybe there's a kcontrol[/kmail] setting available
    KEMailSettings settings;
    if ( !settings.getSetting( KEMailSettings::EmailAddress ).isEmpty() ) {
      mEmailControlCenter = true;
    }
  }
}

void Prefs::Private::setTimeZoneDefault()
{
  KTimeZone zone = KSystemTimeZones::local();
  if ( !zone.isValid() ) {
    kError() << "KSystemTimeZones::local() return 0";
    return;
  }

  kDebug () << "----- time zone:" << zone.name();

  mTimeSpec = zone;
}

Prefs::Prefs() : d( new Private( this ) )
{
  d->init();
}

Prefs::~Prefs()
{
  kDebug();
}

Prefs *Prefs::instance()
{
  if ( !mInstance ) {
    mInstance = new Prefs();

    mInstance->d->readConfig();
  }

  return mInstance;
}

void Prefs::usrSetDefaults()
{
  d->usrSetDefaults();
}

void Prefs::usrReadConfig()
{
  d->usrReadConfig();
}

void Prefs::usrWriteConfig()
{
  d->usrWriteConfig();
}

void Prefs::setMarcusBainsShowSeconds( bool showSeconds )
{
  d->mMarcusBainsShowSeconds = showSeconds;
}

bool Prefs::marcusBainsShowSeconds() const
{
  return d->mMarcusBainsShowSeconds;
}

void Prefs::setAgendaMarcusBainsLineLineColor( const QColor &color )
{
  d->mAgendaMarcusBainsLineLineColor = color;
}

QColor Prefs::agendaMarcusBainsLineLineColor() const
{
  return d->mAgendaMarcusBainsLineLineColor;
}

void Prefs::setMarcusBainsEnabled( bool enabled )
{
  d->mMarcusBainsEnabled = enabled;
}

bool Prefs::marcusBainsEnabled() const
{
  return d->mMarcusBainsEnabled;
}

void Prefs::setAgendaMarcusBainsLineFont( const QFont &font )
{
  d->mAgendaMarcusBainsLineFont = font;
}

QFont Prefs::agendaMarcusBainsLineFont() const
{
  return d->mAgendaMarcusBainsLineFont;
}

void Prefs::setHourSize( int size )
{
  d->mHourSize = size;
}

int Prefs::hourSize() const
{
  return d->mHourSize;
}

void Prefs::setDayBegins( const QDateTime &dateTime )
{
  d->mDayBegins = dateTime;
}

QDateTime Prefs::dayBegins() const
{
  return d->mDayBegins;
}

void Prefs::setWorkingHoursStart( const QDateTime &dateTime )
{
  d->mWorkingHoursStart = dateTime;
}

QDateTime Prefs::workingHoursStart() const
{
  return d->mWorkingHoursStart;
}

void Prefs::setWorkingHoursEnd( const QDateTime &dateTime )
{
  d->mWorkingHoursEnd = dateTime;
}

QDateTime Prefs::workingHoursEnd() const
{
  return d->mWorkingHoursEnd;
}

void Prefs::setSelectionStartsEditor( bool startEditor )
{
  d->mSelectionStartsEditor = startEditor;
}

bool Prefs::selectionStartsEditor() const
{
  return d->mSelectionStartsEditor;
}

void Prefs::setAgendaGridWorkHoursBackgroundColor( const QColor &color )
{
  d->mAgendaGridWorkHoursBackgroundColor = color;
}

QColor Prefs::agendaGridWorkHoursBackgroundColor() const
{
  return d->mAgendaGridWorkHoursBackgroundColor;
}

void Prefs::setAgendaGridHighlightColor( const QColor &color )
{
  d->mAgendaGridHighlightColor = color;
}

QColor Prefs::agendaGridHighlightColor() const
{
  return d->mAgendaGridHighlightColor;
}

void Prefs::setAgendaGridBackgroundColor( const QColor &color )
{
  d->mAgendaGridBackgroundColor = color;
}

QColor Prefs::agendaGridBackgroundColor() const
{
  return d->mAgendaGridBackgroundColor;
}

void Prefs::setEnableAgendaItemIcons( const bool enable )
{
  d->mEnableAgendaItemIcons = enable;
}

bool Prefs::enableAgendaItemIcons() const
{
  return d->mEnableAgendaItemIcons;
}

void Prefs::setTodosUseCategoryColors( bool useColors )
{
  d->mTodosUseCategoryColors = useColors;
}

bool Prefs::todosUseCategoryColors() const
{
  return d->mTodosUseCategoryColors;
}

void Prefs::setAgendaCalendarItemsToDosOverdueBackgroundColor( const QColor &color )
{
  d->mAgendaCalendarItemsToDosOverdueBackgroundColor = color;
}

QColor Prefs::agendaCalendarItemsToDosOverdueBackgroundColor() const
{
  return d->mAgendaCalendarItemsToDosOverdueBackgroundColor;
}

void Prefs::setAgendaCalendarItemsToDosDueTodayBackgroundColor( const QColor &color )
{
  d->mAgendaCalendarItemsToDosDueTodayBackgroundColor = color;
}

QColor Prefs::agendaCalendarItemsToDosDueTodayBackgroundColor() const
{
  return d->mAgendaCalendarItemsToDosDueTodayBackgroundColor;
}

void Prefs::setUnsetCategoryColor( const QColor &color )
{
  d->mUnsetCategoryColor = color;
}

QColor Prefs::unsetCategoryColor() const
{
  return d->mUnsetCategoryColor;
}

void Prefs::setAgendaViewColors( int colors )
{
  d->mAgendaViewColors = colors;
}

int Prefs::agendaViewColors() const
{
  return d->mAgendaViewColors;
}

void Prefs::setAgendaViewFont( const QFont &font )
{
  d->mAgendaViewFont = font;
}

QFont Prefs::agendaViewFont() const
{
  return d->mAgendaViewFont;
}

void Prefs::setEnableToolTips( bool enable )
{
  d->mEnableToolTips = enable;
}

bool Prefs::enableToolTips() const
{
  return d->mEnableToolTips;
}

void Prefs::setDefaultDuration( const QDateTime &dateTime )
{
  d->mDefaultDuration = dateTime;
}

QDateTime Prefs::defaultDuration() const
{
  return d->mDefaultDuration;
}

void Prefs::setShowTodosAgendaView( bool show )
{
  d->mShowTodosAgendaView = show;
}

bool Prefs::showTodosAgendaView() const
{
  return d->mShowTodosAgendaView;
}

void Prefs::setAgendaTimeLabelsFont( const QFont &font )
{
  d->mAgendaTimeLabelsFont = font;
}

QFont Prefs::agendaTimeLabelsFont() const
{
  return d->mAgendaTimeLabelsFont;
}

void Prefs::setWorkWeekMask( int mask )
{
  d->mWorkWeekMask = mask;
}

int Prefs::workWeekMask() const
{
  return d->mWorkWeekMask;
}

void Prefs::setExcludeHolidays( bool exclude )
{
  d->mExcludeHolidays = exclude;
}

bool Prefs::excludeHolidays() const
{
  return d->mExcludeHolidays;
}

void Prefs::setTimeZoneDefault()
{
  d->setTimeZoneDefault();
}

void Prefs::fillMailDefaults()
{
  d->fillMailDefaults();
}

KDateTime::Spec Prefs::timeSpec() const
{
  return KSystemTimeZones::local();
}

void Prefs::setTimeSpec( const KDateTime::Spec &spec )
{
  d->mTimeSpec = spec;
}

void Prefs::setHtmlExportFile( const QString &fileName )
{
  d->mHtmlExportFile = fileName;
}

QString Prefs::htmlExportFile() const
{
  return d->mHtmlExportFile;
}

void Prefs::setPublishPassword( const QString &password )
{
  d->mPublishPassword = password;
}

QString Prefs::publishPassword() const
{
  return d->mPublishPassword;
}

void Prefs::setRetrievePassword( const QString &password )
{
  d->mRetrievePassword = password;
}

QString Prefs::retrievePassword() const
{
  return d->mRetrievePassword;
}

void Prefs::setCategoryColor( const QString &cat, const QColor &color )
{
  d->mCategoryColors.insert( cat, color );
}

QColor Prefs::categoryColor( const QString &cat ) const
{
  QColor color;

  if ( !cat.isEmpty() ) {
    color = d->mCategoryColors.value( cat );
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return d->mDefaultCategoryColor;
  }
}

bool Prefs::hasCategoryColor( const QString &cat ) const
{
    return d->mCategoryColors[ cat ].isValid();
}

QString Prefs::defaultCalendar() const
{
  return d->mDefaultCollection.isValid() ? QString::number( d->mDefaultCollection.id() ) : d->mDefaultCalendar;
}

Akonadi::Collection Prefs::defaultCollection() const
{
  return d->mDefaultCollection;
}

void Prefs::setDefaultCollection( const Akonadi::Collection& col )
{
  d->mDefaultCollection = col;
  if ( !col.isValid() ) {
    d->mDefaultCalendar ="";
  }
}

void Prefs::setResourceColor ( const QString &cal, const QColor &color )
{
  // kDebug() << cal << "color:" << color.name();
  d->mResourceColors.insert( cal, color );
}

QColor Prefs::resourceColor( const QString &cal )
{
  QColor color;
  if ( !cal.isEmpty() ) {
    if ( d->mResourceColors.contains( cal ) ) {
      color = d->mResourceColors.value( cal );
      if ( !color.isValid() )
        return color;
    }
  } else {
    return d->mDefaultResourceColor;
  }

  // assign default color if enabled
  if ( !cal.isEmpty() && !color.isValid() && d->assignDefaultResourceColors() ) {
    QColor defColor( 0x37, 0x7A, 0xBC );
    if ( d->defaultResourceColorSeed() > 0 &&
         d->defaultResourceColorSeed() - 1 < (int)d->defaultResourceColors().size() ) {
        defColor = QColor( d->defaultResourceColors()[d->defaultResourceColorSeed()-1] );
    } else {
        int h, s, v;
        defColor.getHsv( &h, &s, &v );
        h = ( d->defaultResourceColorSeed() % 12 ) * 30;
        s -= s * static_cast<int>( ( ( d->defaultResourceColorSeed() / 12 ) % 2 ) * 0.5 );
        defColor.setHsv( h, s, v );
    }
    d->setDefaultResourceColorSeed( d->defaultResourceColorSeed() + 1 );
    setResourceColor( cal, defColor );
    color = d->mResourceColors[cal];
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return d->mDefaultResourceColor;
  }
}

QString Prefs::fullName() const
{
  QString tusername;
  if ( d->mEmailControlCenter ) {
    KEMailSettings settings;
    tusername = settings.getSetting( KEMailSettings::RealName );
  } else {
    tusername = d->userName();
  }

  // Quote the username as it might contain commas and other quotable chars.
  tusername = KPIMUtils::quoteNameIfNecessary( tusername );

  QString tname, temail;
  // ignore the return value from extractEmailAddressAndName() because
  // it will always be false since tusername does not contain "@domain".
  KPIMUtils::extractEmailAddressAndName( tusername, temail, tname );
  return tname;
}

QString Prefs::email() const
{
  if ( d->mEmailControlCenter ) {
    KEMailSettings settings;
    return settings.getSetting( KEMailSettings::EmailAddress );
  } else {
    return d->userEmail();
  }
}

QStringList Prefs::allEmails() const
{
  // Grab emails from the email identities
  QStringList lst;/* = KOCore::self()->identityManager()->allEmails();
  // Add emails configured in korganizer
  lst += mAdditionalMails;
  // Add the email entered as the userEmail here
  lst += email();
                  */
  // Warning, this list could contain duplicates.
  return lst;
}

QStringList Prefs::fullEmails() const
{
  QStringList fullEmails;
  /*
  // The user name and email from the config dialog:
  fullEmails << QString( "%1 <%2>" ).arg( fullName() ).arg( email() );

  QStringList::Iterator it;
  // Grab emails from the email identities
  IdentityManager *idmanager = KOCore::self()->identityManager();
  QStringList lst = idmanager->identities();
  IdentityManager::ConstIterator it1;
  for ( it1 = idmanager->begin(); it1 != idmanager->end(); ++it1 ) {
    fullEmails << (*it1).fullEmailAddr();
  }
  // Add emails configured in korganizer
  lst = mAdditionalMails;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << QString( "%1 <%2>" ).arg( fullName() ).arg( *it );
  }

  // Warning, this list could contain duplicates.
  // */
  return fullEmails;
}

bool Prefs::thatIsMe(const QString &_email ) const
{
  // TODO_SPLIT: tirar o unused
  Q_UNUSED( _email );
  // NOTE: this method is called for every created agenda view item,
  // so we need to keep performance in mind

  /* identityManager()->thatIsMe() is quite expensive since it does parsing of
     _email in a way which is unnecessarily complex for what we can have here,
     so we do that ourselves. This makes sense since this

  if ( KOCore::self()->identityManager()->thatIsMe( _email ) ) {
    return true;
  }
  */

  // in case email contains a full name, strip it out.
  // the below is the simpler but slower version of the following code:
  // const QString email = KPIM::getEmailAddress( _email );
/*  const QByteArray tmp = _email.toUtf8();
  const char *cursor = tmp.constData();
  const char *end = tmp.data() + tmp.length();
  KMime::Types::Mailbox mbox;
  KMime::HeaderParsing::parseMailbox( cursor, end, mbox );
  const QString email = mbox.addrSpec().asString();

  if ( this->email() == email ) {
    return true;
  }

  for ( IdentityManager::ConstIterator it = KOCore::self()->identityManager()->begin();
        it != KOCore::self()->identityManager()->end(); ++it ) {
    if ( email == (*it).emailAddr() ) {
      return true;
    }
  }

  if ( mAdditionalMails.contains( email ) ) {
    return true;
  }
*/
  return false;
}

QStringList Prefs::timeScaleTimezones() const
{
  return d->mTimeScaleTimeZones;
}

void Prefs::setTimeScaleTimezones( const QStringList &list )
{
  d->mTimeScaleTimeZones = list;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
