/*
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

#include <KGlobalSettings>
#include <KSystemTimeZone>
#include <KDebug>

using namespace EventViews;

QSet<EventViews::EventView::ItemIcon> iconArrayToSet( const QByteArray &array )
{
  QSet<EventViews::EventView::ItemIcon> set;
  for ( int i=0; i<array.count(); ++i ) {
    if ( i >= EventViews::EventView::IconCount ) {
      kWarning() << "Icon array is too big: " << array.count();
      return set;
    }
    if ( array[i] != 0 ) {
      set.insert( static_cast<EventViews::EventView::ItemIcon>( i ) );
    }
  }
  return set;
}

QByteArray iconSetToArray( const QSet<EventViews::EventView::ItemIcon> &set )
{
  QByteArray array;
  for ( int i=0; i<EventViews::EventView::IconCount; ++i ) {
    const bool contains = set.contains( static_cast<EventViews::EventView::ItemIcon>( i ) );
    array.append( contains ? 1 : 0 ) ;
  }

  return array;
}

QByteArray agendaViewIconDefaults()
{
  QByteArray iconDefaults;

  iconDefaults[EventViews::EventView::CalendarCustomIcon] = 1;
  iconDefaults[EventViews::EventView::TaskIcon]           = 1;
  iconDefaults[EventViews::EventView::JournalIcon]        = 1;
  iconDefaults[EventViews::EventView::RecurringIcon]      = 1;
  iconDefaults[EventViews::EventView::ReminderIcon]       = 1;
  iconDefaults[EventViews::EventView::ReadOnlyIcon]       = 1;
  iconDefaults[EventViews::EventView::ReplyIcon]          = 0;

  return iconDefaults;
}

QByteArray monthViewIconDefaults()
{
  QByteArray iconDefaults;

  iconDefaults[EventViews::EventView::CalendarCustomIcon] = 1;
  iconDefaults[EventViews::EventView::TaskIcon]           = 1;
  iconDefaults[EventViews::EventView::JournalIcon]        = 1;
  iconDefaults[EventViews::EventView::RecurringIcon]      = 0;
  iconDefaults[EventViews::EventView::ReminderIcon]       = 0;
  iconDefaults[EventViews::EventView::ReadOnlyIcon]       = 1;
  iconDefaults[EventViews::EventView::ReplyIcon]          = 0;

  return iconDefaults;
}

class BaseConfig : public PrefsBase
{
  public:
    BaseConfig();

    void setResourceColor( const QString &resource, const QColor &color );

    void setTimeScaleTimezones( const QStringList &timeZones );
    QStringList timeScaleTimezones() const;

  public:
    QHash<QString,QColor> mResourceColors;
    QColor mDefaultResourceColor;

    QFont mDefaultMonthViewFont;
    QFont mDefaultAgendaTimeLabelsFont;

    KDateTime::Spec mTimeSpec;
    QStringList mTimeScaleTimeZones;

    QSet<EventViews::EventView::ItemIcon> mAgendaViewIcons;
    QSet<EventViews::EventView::ItemIcon> mMonthViewIcons;

  protected:
    void usrSetDefaults();
    void usrReadConfig();
    bool usrWriteConfig();

    void setTimeZoneDefault();
};

BaseConfig::BaseConfig() : PrefsBase()
{
  mDefaultResourceColor = QColor(); //Default is a color invalid

  mDefaultAgendaTimeLabelsFont = KGlobalSettings::generalFont();
  // make a large default time bar font, at least 16 points.
  mDefaultAgendaTimeLabelsFont.setPointSize(
    qMax( mDefaultAgendaTimeLabelsFont.pointSize() + 4, 16 ) );

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize(
    qMax( mDefaultMonthViewFont.pointSize() - 2, 6 ) );

  agendaTimeLabelsFontItem()->setDefaultValue( mDefaultAgendaTimeLabelsFont );
  agendaTimeLabelsFontItem()->setDefault();
  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );
  monthViewFontItem()->setDefault();
}

void BaseConfig::setResourceColor( const QString &resource, const QColor &color )
{
  mResourceColors.insert( resource, color );
}

void BaseConfig::setTimeScaleTimezones( const QStringList &list )
{
  mTimeScaleTimeZones = list;
}

QStringList BaseConfig::timeScaleTimezones() const
{
  return mTimeScaleTimeZones;
}

void BaseConfig::usrSetDefaults()
{
  setAgendaTimeLabelsFont( mDefaultAgendaTimeLabelsFont );
  setMonthViewFont( mDefaultMonthViewFont );

  setTimeZoneDefault();

  PrefsBase::usrSetDefaults();
}

void BaseConfig::usrReadConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  // Note that the [Category Colors] group was removed after 3.2 due to
  // an algorithm change. That's why we now use [Category Colors2]
  // Resource colors
  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  const QStringList colorKeyList = rColorsConfig.keyList();

  QStringList::ConstIterator it3;
  for ( it3 = colorKeyList.begin(); it3 != colorKeyList.end(); ++it3 ) {
    QColor color = rColorsConfig.readEntry( *it3, mDefaultResourceColor );
    //kDebug() << "key:" << (*it3) << "value:" << color;
    setResourceColor( *it3, color );
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

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  setTimeScaleTimezones( timeScaleConfig.readEntry( "Timescale Timezones", QStringList() ) );

  KConfigGroup monthViewConfig( config(), "Month View" );
  KConfigGroup agendaViewConfig( config(), "Agenda View" );
  const QByteArray agendaIconArray =
    agendaViewConfig.readEntry<QByteArray>( "agendaViewItemIcons", agendaViewIconDefaults() );
  const QByteArray monthIconArray =
    monthViewConfig.readEntry<QByteArray>( "monthViewItemIcons", monthViewIconDefaults() );

  mAgendaViewIcons = iconArrayToSet( agendaIconArray );
  mMonthViewIcons = iconArrayToSet( monthIconArray );

  KConfigSkeleton::usrReadConfig();
}

bool BaseConfig::usrWriteConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  QHash<QString, QColor>::const_iterator i = mResourceColors.constBegin();
  while ( i != mResourceColors.constEnd() ) {
    rColorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

#if 0
  if ( mRememberRetrievePw ) {
    config()->writeEntry( "Retrieve Server Password",
                          KStringHandler::obscure( mRetrievePassword ) );
  } else {
    config()->deleteEntry( "Retrieve Server Password" );
  }
#endif

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  timeScaleConfig.writeEntry( "Timescale Timezones", timeScaleTimezones() );

  KConfigGroup monthViewConfig( config(), "Month View" );
  KConfigGroup agendaViewConfig( config(), "Agenda View" );

  const QByteArray agendaIconArray = iconSetToArray( mAgendaViewIcons );
  const QByteArray monthIconArray = iconSetToArray( mMonthViewIcons );

  agendaViewConfig.writeEntry<QByteArray>( "agendaViewItemIcons", agendaIconArray );
  monthViewConfig.writeEntry<QByteArray>( "monthViewItemIcons", monthIconArray );

  return KConfigSkeleton::usrWriteConfig();
}

void BaseConfig::setTimeZoneDefault()
{
  KTimeZone zone = KSystemTimeZones::local();
  if ( !zone.isValid() ) {
    kError() << "KSystemTimeZones::local() return 0";
    return;
  }

  kDebug () << "----- time zone:" << zone.name();

  mTimeSpec = zone;
}

class Prefs::Private
{
  public:
    Private( Prefs *parent ) : mAppConfig( 0 ), q( parent ) {}
    Private( Prefs *parent, KCoreConfigSkeleton *appConfig )
      : mAppConfig( appConfig ), q( parent ) {}

    void setTimeZoneDefault();

    KConfigSkeletonItem *appConfigItem( const KConfigSkeletonItem *baseConfigItem ) const;

    void setBool( KCoreConfigSkeleton::ItemBool *baseConfigItem, bool value );
    bool getBool( const KCoreConfigSkeleton::ItemBool *baseConfigItem ) const;

    void setInt( KCoreConfigSkeleton::ItemInt *baseConfigItem, int value );
    int getInt( const KCoreConfigSkeleton::ItemInt *baseConfigItem ) const;

    void setString( KCoreConfigSkeleton::ItemString *baseConfigItem, const QString &value );
    QString getString( const KCoreConfigSkeleton::ItemString *baseConfigItem ) const;

    void setDateTime( KCoreConfigSkeleton::ItemDateTime *baseConfigItem, const QDateTime &value );
    QDateTime getDateTime( const KCoreConfigSkeleton::ItemDateTime *baseConfigItem ) const;

    void setStringList( KCoreConfigSkeleton::ItemStringList *baseConfigItem,
                        const QStringList &value );
    QStringList getStringList( const KCoreConfigSkeleton::ItemStringList *baseConfigItem ) const;

    void setColor( KConfigSkeleton::ItemColor *baseConfigItem, const QColor &value );
    QColor getColor( const KConfigSkeleton::ItemColor *baseConfigItem ) const;

    void setFont( KConfigSkeleton::ItemFont *baseConfigItem, const QFont &value );
    QFont getFont( const KConfigSkeleton::ItemFont *baseConfigItem ) const;

  public:
    BaseConfig mBaseConfig;
    KCoreConfigSkeleton *mAppConfig;

  private:
    Prefs *q;
};

KConfigSkeletonItem *Prefs::Private::appConfigItem(
  const KConfigSkeletonItem *baseConfigItem ) const
{
  Q_ASSERT( baseConfigItem );

  if ( mAppConfig ) {
    return mAppConfig->findItem( baseConfigItem->name() );
  }

  return 0;
}

void Prefs::Private::setBool( KCoreConfigSkeleton::ItemBool *baseConfigItem, bool value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemBool *item = dynamic_cast<KCoreConfigSkeleton::ItemBool*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Bool";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

bool Prefs::Private::getBool( const KCoreConfigSkeleton::ItemBool *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemBool *item = dynamic_cast<KCoreConfigSkeleton::ItemBool*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Bool";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setInt( KCoreConfigSkeleton::ItemInt *baseConfigItem, int value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemInt *item = dynamic_cast<KCoreConfigSkeleton::ItemInt*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Int";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

int Prefs::Private::getInt( const KCoreConfigSkeleton::ItemInt *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemInt *item = dynamic_cast<KCoreConfigSkeleton::ItemInt*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Int";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setString( KCoreConfigSkeleton::ItemString *baseConfigItem,
                                const QString &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemString *item =
      dynamic_cast<KCoreConfigSkeleton::ItemString*>( appItem );

    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type String";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QString Prefs::Private::getString( const KCoreConfigSkeleton::ItemString *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemString *item =
      dynamic_cast<KCoreConfigSkeleton::ItemString*>( appItem );

    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type String";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setDateTime( KCoreConfigSkeleton::ItemDateTime *baseConfigItem,
                                  const QDateTime &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemDateTime *item =
      dynamic_cast<KCoreConfigSkeleton::ItemDateTime*>( appItem );

    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type DateTime";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QDateTime Prefs::Private::getDateTime(
  const KCoreConfigSkeleton::ItemDateTime *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemDateTime *item =
      dynamic_cast<KCoreConfigSkeleton::ItemDateTime*>( appItem );

    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type DateTime";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setStringList( KCoreConfigSkeleton::ItemStringList *baseConfigItem,
                                    const QStringList &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemStringList *item =
      dynamic_cast<KCoreConfigSkeleton::ItemStringList*>( appItem );

    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type StringList";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QStringList Prefs::Private::getStringList(
  const KCoreConfigSkeleton::ItemStringList *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KCoreConfigSkeleton::ItemStringList *item =
      dynamic_cast<KCoreConfigSkeleton::ItemStringList*>( appItem );

    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type StringList";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setColor( KConfigSkeleton::ItemColor *baseConfigItem, const QColor &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Color";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QColor Prefs::Private::getColor( const KConfigSkeleton::ItemColor *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemColor *item = dynamic_cast<KConfigSkeleton::ItemColor*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Color";
  }
  return baseConfigItem->value();
}

void Prefs::Private::setFont( KConfigSkeleton::ItemFont *baseConfigItem, const QFont &value )
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( appItem );
    if ( item ) {
      item->setValue( value );
    } else {
      kError() << "Application config item" << appItem->name() << "is not of type Font";
    }
  } else {
    baseConfigItem->setValue( value );
  }
}

QFont Prefs::Private::getFont( const KConfigSkeleton::ItemFont *baseConfigItem ) const
{
  KConfigSkeletonItem *appItem = appConfigItem( baseConfigItem );
  if ( appItem ) {
    KConfigSkeleton::ItemFont *item = dynamic_cast<KConfigSkeleton::ItemFont*>( appItem );
    if ( item ) {
      return item->value();
    }
    kError() << "Application config item" << appItem->name() << "is not of type Font";
  }
  return baseConfigItem->value();
}

Prefs::Prefs() : d( new Private( this ) )
{
}

Prefs::Prefs( KCoreConfigSkeleton *appConfig ) : d( new Private( this, appConfig ) )
{
}

Prefs::~Prefs()
{
  delete d;
}

void Prefs::readConfig()
{
  d->mBaseConfig.readConfig();
  if ( d->mAppConfig ) {
    d->mAppConfig->readConfig();
  }
}

void Prefs::writeConfig()
{
  d->mBaseConfig.writeConfig();
  if ( d->mAppConfig ) {
    d->mAppConfig->writeConfig();
  }
}

void Prefs::setMarcusBainsShowSeconds( bool showSeconds )
{
  d->setBool( d->mBaseConfig.marcusBainsShowSecondsItem(), showSeconds );
}

bool Prefs::marcusBainsShowSeconds() const
{
  return d->getBool( d->mBaseConfig.marcusBainsShowSecondsItem() );
}

void Prefs::setAgendaMarcusBainsLineLineColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaMarcusBainsLineLineColorItem(), color );
}

QColor Prefs::agendaMarcusBainsLineLineColor() const
{
  return d->getColor( d->mBaseConfig.agendaMarcusBainsLineLineColorItem() );
}

void Prefs::setMarcusBainsEnabled( bool enabled )
{
  d->setBool( d->mBaseConfig.marcusBainsEnabledItem(), enabled );
}

bool Prefs::marcusBainsEnabled() const
{
  return d->getBool( d->mBaseConfig.marcusBainsEnabledItem() );
}

void Prefs::setAgendaMarcusBainsLineFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.agendaMarcusBainsLineFontItem(), font );
}

QFont Prefs::agendaMarcusBainsLineFont() const
{
  return d->getFont( d->mBaseConfig.agendaMarcusBainsLineFontItem() );
}

void Prefs::setHourSize( int size )
{
  d->setInt( d->mBaseConfig.hourSizeItem(), size );
}

int Prefs::hourSize() const
{
  return d->getInt( d->mBaseConfig.hourSizeItem() );
}

void Prefs::setDayBegins( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.dayBeginsItem(), dateTime );
}

QDateTime Prefs::dayBegins() const
{
  return d->getDateTime( d->mBaseConfig.dayBeginsItem() );
}

void Prefs::setWorkingHoursStart( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.workingHoursStartItem(), dateTime );
}

QDateTime Prefs::workingHoursStart() const
{
  return d->getDateTime( d->mBaseConfig.workingHoursStartItem() );
}

void Prefs::setWorkingHoursEnd( const QDateTime &dateTime )
{
  d->setDateTime( d->mBaseConfig.workingHoursEndItem(), dateTime );
}

QDateTime Prefs::workingHoursEnd() const
{
  return d->getDateTime( d->mBaseConfig.workingHoursEndItem() );
}

void Prefs::setSelectionStartsEditor( bool startEditor )
{
  d->setBool( d->mBaseConfig.selectionStartsEditorItem(), startEditor );
}

bool Prefs::selectionStartsEditor() const
{
  return d->getBool( d->mBaseConfig.selectionStartsEditorItem() );
}

void Prefs::setAgendaGridWorkHoursBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaGridWorkHoursBackgroundColorItem(), color );
}

QColor Prefs::agendaGridWorkHoursBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaGridWorkHoursBackgroundColorItem() );
}

void Prefs::setAgendaGridHighlightColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaGridHighlightColorItem(), color );
}

QColor Prefs::agendaGridHighlightColor() const
{
  return d->getColor( d->mBaseConfig.agendaGridHighlightColorItem() );
}

void Prefs::setAgendaGridBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.agendaGridBackgroundColorItem(), color );
}

QColor Prefs::agendaGridBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaGridBackgroundColorItem() );
}

void Prefs::setEnableAgendaItemIcons( bool enable )
{
  d->setBool( d->mBaseConfig.enableAgendaItemIconsItem(), enable );
}

bool Prefs::enableAgendaItemIcons() const
{
  return d->getBool( d->mBaseConfig.enableAgendaItemIconsItem() );
}

void Prefs::setTodosUseCategoryColors( bool useColors )
{
  d->setBool( d->mBaseConfig.todosUseCategoryColorsItem(), useColors );
}

bool Prefs::todosUseCategoryColors() const
{
  return d->getBool( d->mBaseConfig.todosUseCategoryColorsItem() );
}

void Prefs::setAgendaHolidaysBackgroundColor( const QColor &color ) const
{
  d->setColor( d->mBaseConfig.agendaHolidaysBackgroundColorItem(), color );
}

QColor Prefs::agendaHolidaysBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaHolidaysBackgroundColorItem() );
}

void Prefs::setAgendaViewColors( int colors )
{
  d->setInt( d->mBaseConfig.agendaViewColorsItem(), colors );
}

int Prefs::agendaViewColors() const
{
  return d->getInt( d->mBaseConfig.agendaViewColorsItem() );
}

void Prefs::setAgendaViewFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.agendaViewFontItem(), font );
}

QFont Prefs::agendaViewFont() const
{
  return d->getFont( d->mBaseConfig.agendaViewFontItem() );
}

void Prefs::setMonthViewFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.monthViewFontItem(), font );
}

QFont Prefs::monthViewFont() const
{
  return d->getFont( d->mBaseConfig.monthViewFontItem() );
}

QColor Prefs::monthGridBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.monthGridBackgroundColorItem() );
}

void Prefs::setMonthGridBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.monthGridBackgroundColorItem(), color );
}

QColor Prefs::monthGridWorkHoursBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.monthGridWorkHoursBackgroundColorItem() );
}

void Prefs::monthGridWorkHoursBackgroundColor( const QColor &color )
{
  d->setColor( d->mBaseConfig.monthGridWorkHoursBackgroundColorItem(), color );
}

int Prefs::monthViewColors() const
{
  return d->getInt( d->mBaseConfig.monthViewColorsItem() );
}

void Prefs::setMonthViewColors( int colors ) const
{
  d->setInt( d->mBaseConfig.monthViewColorsItem(), colors );
}

void Prefs::setEnableMonthItemIcons( bool enable )
{
  d->setBool( d->mBaseConfig.enableMonthItemIconsItem(), enable );
}

bool Prefs::enableMonthItemIcons() const
{
  return d->getBool( d->mBaseConfig.enableMonthItemIconsItem() );
}

bool Prefs::showTimeInMonthView() const
{
  return d->getBool( d->mBaseConfig.showTimeInMonthViewItem() );
}

void Prefs::setShowTimeInMonthView( bool show )
{
  d->setBool( d->mBaseConfig.showTimeInMonthViewItem(), show );
}

bool Prefs::showTodosMonthView() const
{
  return d->getBool( d->mBaseConfig.showTodosMonthViewItem() );
}

void Prefs::setShowTodosMonthView( bool enable )
{
  d->setBool( d->mBaseConfig.showTodosMonthViewItem(), enable );
}

bool Prefs::showJournalsMonthView() const
{
  return d->getBool( d->mBaseConfig.showJournalsMonthViewItem() );
}

void Prefs::setShowJournalsMonthView( bool enable )
{
  d->setBool( d->mBaseConfig.showJournalsMonthViewItem(), enable );
}

bool Prefs::fullViewMonth() const
{
  return d->getBool( d->mBaseConfig.fullViewMonthItem() );
}

void Prefs::setFullViewMonth( bool fullView )
{
  d->setBool( d->mBaseConfig.fullViewMonthItem(), fullView );
}

bool Prefs::sortCompletedTodosSeparately() const
{
  return d->getBool( d->mBaseConfig.sortCompletedTodosSeparatelyItem() );
}

void Prefs::setSortCompletedTodosSeparately( bool enable )
{
  d->setBool( d->mBaseConfig.sortCompletedTodosSeparatelyItem(), enable );
}

void Prefs::setEnableToolTips( bool enable )
{
  d->setBool( d->mBaseConfig.enableToolTipsItem(), enable );
}

bool Prefs::enableToolTips() const
{
  return d->getBool( d->mBaseConfig.enableToolTipsItem() );
}

void Prefs::setShowTodosAgendaView( bool show )
{
  d->setBool( d->mBaseConfig.showTodosAgendaViewItem(), show );
}

bool Prefs::showTodosAgendaView() const
{
  return d->getBool( d->mBaseConfig.showTodosAgendaViewItem() );
}

void Prefs::setAgendaTimeLabelsFont( const QFont &font )
{
  d->setFont( d->mBaseConfig.agendaTimeLabelsFontItem(), font );
}

QFont Prefs::agendaTimeLabelsFont() const
{
  return d->getFont( d->mBaseConfig.agendaTimeLabelsFontItem() );
}

KDateTime::Spec Prefs::timeSpec() const
{
  return KSystemTimeZones::local();
}

void Prefs::setTimeSpec( const KDateTime::Spec &spec )
{
  d->mBaseConfig.mTimeSpec = spec;
}

bool Prefs::colorAgendaBusyDays() const
{
  return d->getBool( d->mBaseConfig.colorBusyDaysEnabledItem() );
}

bool Prefs::colorMonthBusyDays() const
{
  return d->getBool( d->mBaseConfig.colorMonthBusyDaysEnabledItem() );
}

QColor Prefs::viewBgBusyColor() const
{
  return d->getColor( d->mBaseConfig.viewBgBusyColorItem() );
}

void Prefs::setViewBgBusyColor( const QColor &color )
{
  d->mBaseConfig.mViewBgBusyColor = color;
}

QColor Prefs::holidayColor() const
{
  return d->getColor( d->mBaseConfig.holidayColorItem() );
}

void Prefs::setHolidayColor( const QColor &color )
{
  d->mBaseConfig.mHolidayColor = color;
}

QColor Prefs::agendaViewBackgroundColor() const
{
  return d->getColor( d->mBaseConfig.agendaBgColorItem() );
}

void Prefs::setAgendaViewBackgroundColor( const QColor &color )
{
  d->mBaseConfig.mAgendaBgColor = color;
}

QColor Prefs::workingHoursColor() const
{
  return d->getColor( d->mBaseConfig.workingHoursColorItem() );
}

void Prefs::setWorkingHoursColor( const QColor &color )
{
  d->mBaseConfig.mWorkingHoursColor = color;
}

QColor Prefs::todoDueTodayColor() const
{
  return d->getColor( d->mBaseConfig.todoDueTodayColorItem() );
}

void Prefs::setTodoDueTodayColor( const QColor &color )
{
  d->mBaseConfig.mTodoDueTodayColor = color;
}

QColor Prefs::todoOverdueColor() const
{
  return d->getColor( d->mBaseConfig.todoOverdueColorItem() );
}

void Prefs::setTodoOverdueColor( const QColor &color )
{
  d->mBaseConfig.mTodoOverdueColor = color;
}

void Prefs::setColorAgendaBusyDays( bool enable )
{
  d->mBaseConfig.mColorBusyDaysEnabled = enable;
}

void Prefs::setColorMonthBusyDays( bool enable )
{
  d->mBaseConfig.mColorMonthBusyDaysEnabled = enable;
}

void Prefs::setResourceColor ( const QString &cal, const QColor &color )
{
  d->mBaseConfig.setResourceColor( cal, color );
}

QColor Prefs::resourceColor( const QString &cal )
{
  QColor color;
  if ( !cal.isEmpty() ) {
    if ( d->mBaseConfig.mResourceColors.contains( cal ) ) {
      color = d->mBaseConfig.mResourceColors.value( cal );
      if ( !color.isValid() ) {
        return color;
      }
    }
  } else {
    return d->mBaseConfig.mDefaultResourceColor;
  }

  // assign default color if enabled
  if ( !cal.isEmpty() && !color.isValid() &&
       d->getBool( d->mBaseConfig.assignDefaultResourceColorsItem() ) ) {
    QColor defColor( 0x37, 0x7A, 0xBC );
    const int seed = d->getInt( d->mBaseConfig.defaultResourceColorSeedItem() );
    const QStringList colors = d->getStringList( d->mBaseConfig.defaultResourceColorsItem() );
    if ( seed > 0 && seed - 1 < (int)colors.size() ) {
        defColor = QColor( colors[seed-1] );
    } else {
        int h, s, v;
        defColor.getHsv( &h, &s, &v );
        h = ( seed % 12 ) * 30;
        s -= s * static_cast<int>( ( ( seed / 12 ) % 2 ) * 0.5 );
        defColor.setHsv( h, s, v );
    }
    d->setInt( d->mBaseConfig.defaultResourceColorSeedItem(), ( seed + 1 ) );
    d->mBaseConfig.setResourceColor( cal, defColor );
    color = d->mBaseConfig.mResourceColors[cal];
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return d->mBaseConfig.mDefaultResourceColor;
  }
}

QStringList Prefs::timeScaleTimezones() const
{
  return d->mBaseConfig.timeScaleTimezones();
}

void Prefs::setTimeScaleTimezones( const QStringList &list )
{
  d->mBaseConfig.setTimeScaleTimezones( list );
}

KConfigSkeleton::ItemFont *Prefs::fontItem( const QString &name ) const
{
  KConfigSkeletonItem *item = d->mAppConfig ? d->mAppConfig->findItem( name ) : 0;

  if ( !item ) {
    item = d->mBaseConfig.findItem( name );
  }

  return dynamic_cast<KConfigSkeleton::ItemFont *>( item );
}

QStringList Prefs::selectedPlugins() const
{
  return d->mBaseConfig.mSelectedPlugins;
}

QStringList Prefs::decorationsAtAgendaViewTop() const
{
  return d->mBaseConfig.decorationsAtAgendaViewTop();
}

QStringList Prefs::decorationsAtAgendaViewBottom() const
{
  return d->mBaseConfig.decorationsAtAgendaViewBottom();
}

void Prefs::setSelectedPlugins( const QStringList &plugins )
{
  d->mBaseConfig.setSelectedPlugins( plugins );
}

void Prefs::setDecorationsAtAgendaViewTop( const QStringList &decorations )
{
  d->mBaseConfig.setDecorationsAtAgendaViewTop( decorations );
}

void Prefs::setDecorationsAtAgendaViewBottom( const QStringList &decorations )
{
  d->mBaseConfig.setDecorationsAtAgendaViewBottom( decorations );
}

QSet<EventViews::EventView::ItemIcon> Prefs::agendaViewIcons() const
{
  return d->mBaseConfig.mAgendaViewIcons;
}

void Prefs::setAgendaViewIcons( const QSet<EventViews::EventView::ItemIcon> &icons )
{
  d->mBaseConfig.mAgendaViewIcons = icons;
}

QSet<EventViews::EventView::ItemIcon> Prefs::monthViewIcons() const
{
  return d->mBaseConfig.mMonthViewIcons;
}

void Prefs::setMonthViewIcons( const QSet<EventViews::EventView::ItemIcon> &icons )
{
  d->mBaseConfig.mMonthViewIcons = icons;
}

void Prefs::setFlatListTodo( bool enable )
{
  d->mBaseConfig.mFlatListTodo = enable;
}

bool Prefs::flatListTodo() const
{
  return d->mBaseConfig.mFlatListTodo;
}

void Prefs::setFullViewTodo( bool enable )
{
  d->mBaseConfig.mFullViewTodo = enable;
}

bool Prefs::fullViewTodo() const
{
  return d->mBaseConfig.mFullViewTodo;
}

bool Prefs::enableTodoQuickSearch() const
{
  return d->mBaseConfig.mEnableTodoQuickSearch;
}

void Prefs::setEnableTodoQuickSearch( bool enable )
{
  d->mBaseConfig.mEnableTodoQuickSearch = enable;
}

bool Prefs::enableQuickTodo() const
{
  return d->mBaseConfig.mEnableQuickTodo;
}

void Prefs::setEnableQuickTodo( bool enable )
{
  d->mBaseConfig.mEnableQuickTodo = enable;
}

bool Prefs::highlightTodos() const
{
  return d->mBaseConfig.mHighlightTodos;
}

void Prefs::setHighlightTodos( bool highlight )
{
  d->mBaseConfig.mHighlightTodos = highlight;
}

KConfig *Prefs::config() const
{
  return d->mAppConfig ? d->mAppConfig->config() : d->mBaseConfig.config();
}
