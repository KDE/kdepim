/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "core/manager.h"

#include "core/aggregation.h"
#include "core/theme.h"
#include "core/view.h"
#include "core/widgetbase.h"
#include "core/storagemodelbase.h"
#include "core/model.h"
#include "core/model_p.h"
#include "core/settings.h"

#include "utils/configureaggregationsdialog.h"
#include "utils/configureaggregationsdialog_p.h"
#include "utils/configurethemesdialog.h"
#include "utils/configurethemesdialog_p.h"

#include "messagecore/settings/globalsettings.h"

#include "messagelistutil.h"

#include <QPixmap>

#include <kmime/kmime_dateformatter.h> // kdepimlibs

#include <KConfig>
#include <KDebug>
#include <KIconLoader>
#include <KGlobalSettings>
#include <KApplication>
#include <KLocalizedString>
#include <KStandardDirs>

using namespace MessageList::Core;

Manager * Manager::mInstance = 0;

Manager::Manager()
  : QObject()
{
  mInstance = this;

  mDateFormatter = new KMime::DateFormatter();

  mPixmapMessageNew = new QPixmap( SmallIcon( QLatin1String( "mail-unread-new" ) ) );
  mPixmapMessageUnread = new QPixmap( SmallIcon( QLatin1String( "mail-unread" ) ) );
  mPixmapMessageRead = new QPixmap( SmallIcon( QLatin1String( "mail-read" ) ) );
  mPixmapMessageDeleted = new QPixmap( SmallIcon( QLatin1String( "mail-deleted" ) ) );
  mPixmapMessageReplied = new QPixmap( SmallIcon( QLatin1String( "mail-replied" ) ) );
  mPixmapMessageRepliedAndForwarded = new QPixmap( SmallIcon( QLatin1String( "mail-forwarded-replied" ) ) );
  mPixmapMessageQueued = new QPixmap( SmallIcon( QLatin1String( "mail-queued" ) ) ); // mail-queue ?
  mPixmapMessageActionItem = new QPixmap( SmallIcon( QLatin1String( "mail-task" ) ) );
  mPixmapMessageSent = new QPixmap( SmallIcon( QLatin1String( "mail-sent" ) ) );
  mPixmapMessageForwarded = new QPixmap( SmallIcon( QLatin1String( "mail-forwarded" ) ) );
  mPixmapMessageImportant = new QPixmap( SmallIcon( QLatin1String( "emblem-important" ) ) ); // "flag"
  mPixmapMessageWatched = new QPixmap( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-thread-watch.png" ) ) );
  mPixmapMessageIgnored = new QPixmap( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-thread-ignored.png" ) ) );
  mPixmapMessageSpam = new QPixmap( SmallIcon( QLatin1String( "mail-mark-junk" ) ) );
  mPixmapMessageHam = new QPixmap( SmallIcon( QLatin1String( "mail-mark-notjunk" ) ) );
  mPixmapMessageFullySigned = new QPixmap( SmallIcon( QLatin1String( "mail-signed-verified" ) ) );
  mPixmapMessagePartiallySigned = new QPixmap( SmallIcon( QLatin1String( "mail-signed-part" ) ) );
  mPixmapMessageUndefinedSigned = new QPixmap( SmallIcon( QLatin1String( "mail-signed" ) ) );
  mPixmapMessageNotSigned = new QPixmap( SmallIcon( QLatin1String( "text-plain" ) ) );
  mPixmapMessageFullyEncrypted = new QPixmap( SmallIcon( QLatin1String( "mail-encrypted-full" ) ) );
  mPixmapMessagePartiallyEncrypted = new QPixmap( SmallIcon( QLatin1String( "mail-encrypted-part" ) ) );
  mPixmapMessageUndefinedEncrypted = new QPixmap( SmallIcon( QLatin1String( "mail-encrypted" ) ) );
  mPixmapMessageNotEncrypted = new QPixmap( SmallIcon( QLatin1String( "text-plain" ) ) );
  mPixmapMessageAttachment = new QPixmap( SmallIcon( QLatin1String( "mail-attachment" ) ) );
  mPixmapMessageAnnotation = new QPixmap( SmallIcon( QLatin1String( "view-pim-notes" ) ) );
  mPixmapMessageInvitation = new QPixmap( SmallIcon( QLatin1String( "mail-invitation" ) ) );
  if ( KApplication::isRightToLeft() )
    mPixmapShowMore = new QPixmap( SmallIcon( QLatin1String( "arrow-left" ) ) );
  else
    mPixmapShowMore = new QPixmap( SmallIcon( QLatin1String( "arrow-right" ) ) );
  mPixmapShowLess = new QPixmap( SmallIcon( QLatin1String( "arrow-down" ) ) );
  mPixmapVerticalLine = new QPixmap( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-vertical-separator-line.png" ) ) );
  mPixmapHorizontalSpacer = new QPixmap( KStandardDirs::locate( "data", QLatin1String( "messagelist/pics/mail-horizontal-space.png" ) ) );

  mCachedLocalizedUnknownText = i18nc( "Unknown date", "Unknown" ) ;

  loadConfiguration();
  connect( Settings::self(), SIGNAL(configChanged()),
           this, SLOT(reloadGlobalConfiguration()) );
}

Manager::~Manager()
{
  saveConfiguration();
  removeAllAggregations();
  removeAllThemes();

  delete mPixmapMessageNew;
  delete mPixmapMessageUnread;
  delete mPixmapMessageRead;
  delete mPixmapMessageDeleted;
  delete mPixmapMessageReplied;
  delete mPixmapMessageRepliedAndForwarded;
  delete mPixmapMessageQueued;
  delete mPixmapMessageActionItem;
  delete mPixmapMessageSent;
  delete mPixmapMessageForwarded;
  delete mPixmapMessageImportant; // "flag"
  delete mPixmapMessageWatched;
  delete mPixmapMessageIgnored;
  delete mPixmapMessageSpam;
  delete mPixmapMessageHam;
  delete mPixmapMessageFullySigned;
  delete mPixmapMessagePartiallySigned;
  delete mPixmapMessageUndefinedSigned;
  delete mPixmapMessageNotSigned;
  delete mPixmapMessageFullyEncrypted;
  delete mPixmapMessagePartiallyEncrypted;
  delete mPixmapMessageUndefinedEncrypted;
  delete mPixmapMessageNotEncrypted;
  delete mPixmapMessageAttachment;
  delete mPixmapMessageAnnotation;
  delete mPixmapMessageInvitation;
  delete mPixmapShowMore;
  delete mPixmapShowLess;
  delete mPixmapVerticalLine;
  delete mPixmapHorizontalSpacer;

  delete mDateFormatter;

  mInstance = 0;
}

void Manager::registerWidget( Widget *pWidget )
{
  if ( !mInstance )
    mInstance = new Manager();

  mInstance->mWidgetList.append( pWidget );
}

void Manager::unregisterWidget( Widget *pWidget )
{
  if ( !mInstance )
  {
    qWarning("ERROR: MessageList::Manager::unregisterWidget() called when Manager::mInstance is 0");
    return;
  }

  mInstance->mWidgetList.removeAll( pWidget );

  if ( mInstance->mWidgetList.isEmpty() )
  {
    delete mInstance;
    mInstance = 0;
  }
}

const Aggregation * Manager::aggregation( const QString &id )
{
  Aggregation * opt = mAggregations.value( id );
  if ( opt )
    return opt;

  return defaultAggregation();
}

const Aggregation * Manager::defaultAggregation()
{
  KConfigGroup conf( Settings::self()->config(),
                     MessageList::Util::storageModelAggregationsGroup() );

  const QString aggregationId = conf.readEntry( QLatin1String( "DefaultSet" ), "" );

  Aggregation * opt = 0;

  if ( !aggregationId.isEmpty() )
    opt = mAggregations.value( aggregationId );

  if ( opt )
    return opt;

  // try just the first one
  QHash< QString, Aggregation * >::ConstIterator it = mAggregations.constBegin();
  if ( it != mAggregations.constEnd() )
    return *it;

  // aargh
  createDefaultAggregations();

  return *( mAggregations.constBegin() );
}

void Manager::saveAggregationForStorageModel( const Akonadi::Collection &col, const QString &id, bool storageUsesPrivateAggregation )
{
  if ( !col.isValid() )
    return;
  saveAggregationForStorageModel( QString::number( col.id() ), id, storageUsesPrivateAggregation );
}

void Manager::saveAggregationForStorageModel( const StorageModel *storageModel, const QString &id, bool storageUsesPrivateAggregation )
{
  saveAggregationForStorageModel( storageModel->id(), id, storageUsesPrivateAggregation );
}

void Manager::saveAggregationForStorageModel( const QString &modelId, const QString &id, bool storageUsesPrivateAggregation )
{
  KConfigGroup conf( Settings::self()->config(),
                     MessageList::Util::storageModelAggregationsGroup() );

  if ( storageUsesPrivateAggregation )
    conf.writeEntry( MessageList::Util::setForStorageModelConfigName().arg( modelId ), id );
  else
    conf.deleteEntry( MessageList::Util::setForStorageModelConfigName().arg( modelId ) );

  if ( !storageUsesPrivateAggregation )
    conf.writeEntry( QLatin1String( "DefaultSet" ), id );
}

const Aggregation * Manager::aggregationForStorageModel( const Akonadi::Collection &col, bool *storageUsesPrivateAggregation )
{
  Q_ASSERT( storageUsesPrivateAggregation );

  *storageUsesPrivateAggregation = false; // this is by default

  if ( !col.isValid() )
    return defaultAggregation();
  return Manager::aggregationForStorageModel( QString::number( col.id() ), storageUsesPrivateAggregation );
}


const Aggregation * Manager::aggregationForStorageModel( const StorageModel *storageModel, bool *storageUsesPrivateAggregation )
{
  Q_ASSERT( storageUsesPrivateAggregation );

  *storageUsesPrivateAggregation = false; // this is by default

  if ( !storageModel )
    return defaultAggregation();
  return Manager::aggregationForStorageModel( storageModel->id(), storageUsesPrivateAggregation );
}

const Aggregation * Manager::aggregationForStorageModel( const QString &storageId, bool *storageUsesPrivateAggregation )
{
  KConfigGroup conf( Settings::self()->config(),
                     MessageList::Util::storageModelAggregationsGroup() );

  const QString aggregationId = conf.readEntry( MessageList::Util::setForStorageModelConfigName().arg( storageId ), "" );

  Aggregation * opt = 0;

  if ( !aggregationId.isEmpty() )
  {
    // a private aggregation was stored
    opt = mAggregations.value( aggregationId );
    *storageUsesPrivateAggregation = ( opt != 0 );
  }

  if ( opt )
    return opt;

  // FIXME: If the storageModel is a mailing list, maybe suggest a mailing-list like preset...
  //        We could even try to guess if the storageModel is a mailing list

  return defaultAggregation();
}

void Manager::addAggregation( Aggregation *set )
{
  Aggregation * old = mAggregations.value( set->id() );
  delete old;
  mAggregations.insert( set->id(), set );
}

void Manager::createDefaultAggregations()
{
  addAggregation(
      new Aggregation(
          i18n( "Current Activity, Threaded" ),
          i18n( "This view uses smart date range groups. " \
                "Messages are threaded. " \
                "So for example, in \"Today\" you will find all the messages arrived today " \
                "and all the threads that have been active today."
            ),
          Aggregation::GroupByDateRange,
          Aggregation::ExpandRecentGroups,
          Aggregation::PerfectReferencesAndSubject,
          Aggregation::MostRecentMessage,
          Aggregation::ExpandThreadsWithUnreadOrImportantMessages,
          Aggregation::FavorInteractivity,
          true
       )
    );

  addAggregation(
      new Aggregation(
          i18n( "Current Activity, Flat" ),
          i18n( "This view uses smart date range groups. " \
                "Messages are not threaded. " \
                "So for example, in \"Today\" you will simply find all the messages arrived today."
            ),
          Aggregation::GroupByDateRange,
          Aggregation::ExpandRecentGroups,
          Aggregation::NoThreading,
          Aggregation::MostRecentMessage,
          Aggregation::NeverExpandThreads,
          Aggregation::FavorInteractivity,
          true
       )
    );

  addAggregation(
      new Aggregation(
          i18n( "Activity by Date, Threaded" ),
          i18n( "This view uses day-by-day groups. " \
                "Messages are threaded. " \
                "So for example, in \"Today\" you will find all the messages arrived today " \
                "and all the threads that have been active today."
            ),
          Aggregation::GroupByDate,
          Aggregation::ExpandRecentGroups,
          Aggregation::PerfectReferencesAndSubject,
          Aggregation::MostRecentMessage,
          Aggregation::ExpandThreadsWithUnreadOrImportantMessages,
          Aggregation::FavorInteractivity,
          true
       )
    );

  addAggregation(
      new Aggregation(
          i18n( "Activity by Date, Flat" ),
          i18n( "This view uses day-by-day groups. " \
                "Messages are not threaded. " \
                "So for example, in \"Today\" you will simply find all the messages arrived today."
            ),
          Aggregation::GroupByDate,
          Aggregation::ExpandRecentGroups,
          Aggregation::NoThreading,
          Aggregation::MostRecentMessage,
          Aggregation::NeverExpandThreads,
          Aggregation::FavorInteractivity,
          true
       )
    );

  addAggregation(
      new Aggregation(
          i18n( "Standard Mailing List" ),
          i18n( "This is a plain and old mailing list view: no groups and heavy threading." ),
          Aggregation::NoGrouping,
          Aggregation::NeverExpandGroups,
          Aggregation::PerfectReferencesAndSubject,
          Aggregation::TopmostMessage,
          Aggregation::ExpandThreadsWithUnreadOrImportantMessages,
          Aggregation::FavorInteractivity,
          true
       )
    );

  addAggregation(
      new Aggregation(
          i18n( "Flat Date View" ),
          i18n( "This is a plain and old list of messages sorted by date: no groups and no threading." \
            ),
          Aggregation::NoGrouping,
          Aggregation::NeverExpandGroups,
          Aggregation::NoThreading,
          Aggregation::TopmostMessage,
          Aggregation::NeverExpandThreads,
          Aggregation::FavorInteractivity,
          true

        )
    );

  addAggregation(
      new Aggregation(
          i18n( "Senders/Receivers, Flat" ),
          i18n( "This view groups the messages by senders or receivers (depending on the folder " \
                "type). " \
                "Messages are not threaded."
            ),
          Aggregation::GroupBySenderOrReceiver,
          Aggregation::NeverExpandGroups,
          Aggregation::NoThreading,
          Aggregation::TopmostMessage,
          Aggregation::NeverExpandThreads,
          Aggregation::FavorSpeed,
          true
        )
    );

  addAggregation(
      new Aggregation(
          i18n( "Thread Starters" ),
          i18n( "This view groups the messages in threads and then groups the threads by the starting user." ),
          Aggregation::GroupBySenderOrReceiver,
          Aggregation::NeverExpandGroups,
          Aggregation::PerfectReferencesAndSubject,
          Aggregation::TopmostMessage,
          Aggregation::NeverExpandThreads,
          Aggregation::FavorSpeed,
          true
        )
    );

/*
  FIX THIS
  addAggregation(
      new Aggregation(
          i18n( "Recent Thread Starters" ),
          i18n( "This view groups the messages in threads and then groups the threads by the starting user. " \
                "Groups are sorted by the date of the first thread start. "
            ),
          Aggregation::GroupBySenderOrReceiver,
          Aggregation::SortGroupsByDateTimeOfMostRecent,
          Aggregation::Descending,
          Aggregation::PerfectReferencesAndSubject,
          Aggregation::TopmostMessage,
          Aggregation::SortMessagesByDateTime,
          Aggregation::Descending
        )
    );
*/
}

void Manager::removeAllAggregations()
{
  QHash< QString, Aggregation * >::ConstIterator end( mAggregations.constEnd() );
  for( QHash< QString, Aggregation * >::ConstIterator it = mAggregations.constBegin(); it != end; ++it )
    delete ( *it );

  mAggregations.clear();
}

void Manager::aggregationsConfigurationCompleted()
{
  if ( mAggregations.isEmpty() )
    createDefaultAggregations(); // panic

  saveConfiguration(); // just to be sure :)

  // notify all the widgets that they should reload the option set combos
  emit aggregationsChanged();
}

const SortOrder Manager::sortOrderForStorageModel( const StorageModel *storageModel, bool *storageUsesPrivateSortOrder )
{
  Q_ASSERT( storageUsesPrivateSortOrder );

  *storageUsesPrivateSortOrder = false; // this is by default

  if ( !storageModel )
    return SortOrder();

  KConfigGroup conf( Settings::self()->config(), MessageList::Util::storageModelSortOrderGroup() );
  SortOrder ret;
  ret.readConfig( conf, storageModel->id(), storageUsesPrivateSortOrder );
  return ret;
}

void Manager::saveSortOrderForStorageModel( const StorageModel *storageModel,
                                            const SortOrder& order, bool storageUsesPrivateSortOrder )
{
  KConfigGroup conf( Settings::self()->config(), MessageList::Util::storageModelSortOrderGroup() );
  order.writeConfig( conf, storageModel->id(), storageUsesPrivateSortOrder );
}

const Theme * Manager::theme( const QString &id )
{
  Theme * opt = mThemes.value( id );
  if ( opt )
    return opt;

  return defaultTheme();
}

const Theme * Manager::defaultTheme()
{
  KConfigGroup conf( Settings::self()->config(), MessageList::Util::storageModelThemesGroup() );

  const QString themeId = conf.readEntry( QLatin1String( "DefaultSet" ), "" );

  Theme * opt = 0;

  if ( !themeId.isEmpty() )
    opt = mThemes.value( themeId );

  if ( opt )
    return opt;

  // try just the first one
  QHash< QString, Theme * >::ConstIterator it = mThemes.constBegin();
  if ( it != mThemes.constEnd() )
    return *it;

  // aargh
  createDefaultThemes();

  it = mThemes.constBegin();

  Q_ASSERT( it != mThemes.constEnd() );

  return *it;
}

void Manager::saveThemeForStorageModel( int index, const QString &id, bool storageUsesPrivateTheme )
{
  saveThemeForStorageModel( QString::number( index ), id, storageUsesPrivateTheme );
}

void Manager::saveThemeForStorageModel( const StorageModel *storageModel, const QString &id, bool storageUsesPrivateTheme )
{
  saveThemeForStorageModel( storageModel->id(), id, storageUsesPrivateTheme );
}

void Manager::saveThemeForStorageModel( const QString &storageModelIndex, const QString &id, bool storageUsesPrivateTheme )
{
  KConfigGroup conf( Settings::self()->config(), MessageList::Util::storageModelThemesGroup() );

  if ( storageUsesPrivateTheme )
    conf.writeEntry( MessageList::Util::setForStorageModelConfigName().arg( storageModelIndex ), id );
  else
    conf.deleteEntry( MessageList::Util::setForStorageModelConfigName().arg( storageModelIndex ) );

  if ( !storageUsesPrivateTheme )
    conf.writeEntry( QLatin1String( "DefaultSet" ), id );
}


const Theme * Manager::themeForStorageModel( const Akonadi::Collection & col,  bool * storageUsesPrivateTheme )
{
  Q_ASSERT( storageUsesPrivateTheme );

  *storageUsesPrivateTheme = false; // this is by default

  if ( !col.isValid() )
    return defaultTheme();
  return Manager::themeForStorageModel( QString::number( col.id() ), storageUsesPrivateTheme );

}

const Theme * Manager::themeForStorageModel( const StorageModel *storageModel, bool *storageUsesPrivateTheme )
{


  Q_ASSERT( storageUsesPrivateTheme );

  *storageUsesPrivateTheme = false; // this is by default

  if ( !storageModel )
    return defaultTheme();
  return Manager::themeForStorageModel( storageModel->id(), storageUsesPrivateTheme );
}

const Theme * Manager::themeForStorageModel( const QString &id, bool *storageUsesPrivateTheme )
{
  KConfigGroup conf( Settings::self()->config(), MessageList::Util::storageModelThemesGroup() );
  const QString themeId = conf.readEntry( MessageList::Util::setForStorageModelConfigName().arg( id ), "" );

  Theme * opt = 0;

  if ( !themeId.isEmpty() )
  {
    // a private theme was stored
    opt = mThemes.value( themeId );
    *storageUsesPrivateTheme = (opt != 0);
  }

  if ( opt )
    return opt;

  // FIXME: If the storageModel is a mailing list, maybe suggest a mailing-list like preset...
  //        We could even try to guess if the storageModel is a mailing list

  // FIXME: Prefer right-to-left themes when application layout is RTL.

  return defaultTheme();
}

void Manager::addTheme( Theme *set )
{
  Theme * old = mThemes.value( set->id() );
  delete old;
  mThemes.insert( set->id(), set );
}

static Theme::Column * add_theme_simple_text_column( Theme * s, const QString &name, Theme::ContentItem::Type type, bool visibleByDefault, SortOrder::MessageSorting messageSorting, bool alignRight, bool addGroupHeaderItem )
{
  Theme::Column * c = new Theme::Column();
  c->setLabel( name );
  c->setVisibleByDefault( visibleByDefault );
  c->setMessageSorting( messageSorting );

  Theme::Row * r = new Theme::Row();

  Theme::ContentItem * i = new Theme::ContentItem( type );
  i->setFont( KGlobalSettings::generalFont() );

  if ( alignRight )
    r->addRightItem( i );
  else
    r->addLeftItem( i );

  c->addMessageRow( r );

  if ( addGroupHeaderItem )
  {
    Theme::Row * r = new Theme::Row();

    Theme::ContentItem * i = new Theme::ContentItem( type );
    i->setFont( KGlobalSettings::generalFont() );

    if ( alignRight )
      r->addRightItem( i );
    else
      r->addLeftItem( i );

    c->addGroupHeaderRow( r );
  }


  s->addColumn( c );

  return c;
}

static Theme::Column * add_theme_simple_icon_column( Theme * s, const QString &name, const QString &pixmapName, Theme::ContentItem::Type type, bool visibleByDefault, SortOrder::MessageSorting messageSorting )
{
  Theme::Column * c = new Theme::Column();
  c->setLabel( name );
  c->setPixmapName( pixmapName );
  c->setVisibleByDefault( visibleByDefault );
  c->setMessageSorting( messageSorting );

  Theme::Row * r = new Theme::Row();

  Theme::ContentItem * i = new Theme::ContentItem( type );
  i->setSoftenByBlendingWhenDisabled( true );

  r->addLeftItem( i );

  c->addMessageRow( r );

  s->addColumn( c );

  return c;
}


void Manager::createDefaultThemes()
{
  Theme * s;
  Theme::Column * c;
  Theme::Row * r;
  Theme::ContentItem * i;

  // The "Classic" backward compatible theme

  s = new Theme(
      i18nc( "Default theme name", "Classic" ),
      i18n( "A simple, backward compatible, single row theme" ), true /*readOnly*/
    );

    c = new Theme::Column();
    c->setLabel( i18nc( "@title:column Subject of messages", "Subject" ) );
    c->setMessageSorting( SortOrder::SortMessagesBySubject );

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::ExpandedStateIcon );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::GroupHeaderLabel );
        QFont bigFont = KGlobalSettings::generalFont();
        bigFont.setBold( true );
        i->setFont( bigFont );
        i->setUseCustomFont( true );
      r->addLeftItem( i );
    c->addGroupHeaderRow( r );

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::CombinedReadRepliedStateIcon );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::AttachmentStateIcon );
        i->setHideWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::AnnotationIcon );
        i->setHideWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::InvitationIcon );
        i->setHideWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::SignatureStateIcon );
        i->setHideWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::EncryptionStateIcon );
        i->setHideWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::Subject );
      r->addLeftItem( i );
    c->addMessageRow( r );

  s->addColumn( c );

  c = add_theme_simple_text_column( s, i18n( "Sender/Receiver" ), Theme::ContentItem::SenderOrReceiver, true, SortOrder::SortMessagesBySenderOrReceiver, false, false);
  c->setIsSenderOrReceiver( true );
  add_theme_simple_text_column( s, i18nc( "Sender of a message", "Sender" ), Theme::ContentItem::Sender, false, SortOrder::SortMessagesBySender, false, false );
  add_theme_simple_text_column( s, i18nc( "Receiver of a message", "Receiver" ), Theme::ContentItem::Receiver, false, SortOrder::SortMessagesByReceiver, false, false );
  add_theme_simple_text_column( s, i18nc( "Date of a message", "Date" ), Theme::ContentItem::Date, true, SortOrder::SortMessagesByDateTime, false, false );
  add_theme_simple_text_column( s, i18n( "Most Recent Date" ), Theme::ContentItem::MostRecentDate, false, SortOrder::SortMessagesByDateTimeOfMostRecent, false, true );
  add_theme_simple_text_column( s, i18nc( "Size of a message", "Size" ), Theme::ContentItem::Size, false, SortOrder::SortMessagesBySize, false, false );
  add_theme_simple_icon_column( s, i18nc( "Attachement indication", "Attachment" ), QLatin1String( "mail-attachment" ), Theme::ContentItem::AttachmentStateIcon, false, SortOrder::NoMessageSorting );
  add_theme_simple_icon_column( s, i18n( "Read/Unread" ), QLatin1String( "mail-unread-new" ), Theme::ContentItem::ReadStateIcon, false, SortOrder::SortMessagesByUnreadStatus );
  add_theme_simple_icon_column( s, i18n( "Replied" ), QLatin1String( "mail-replied" ), Theme::ContentItem::RepliedStateIcon, false, SortOrder::NoMessageSorting );
  add_theme_simple_icon_column( s, i18nc( "Message importance indication", "Important" ), QLatin1String( "emblem-important" ), Theme::ContentItem::ImportantStateIcon, false, SortOrder::SortMessagesByImportantStatus );
  add_theme_simple_icon_column( s, i18n( "Action Item" ), QLatin1String( "mail-task" ), Theme::ContentItem::ActionItemStateIcon, false, SortOrder::SortMessagesByActionItemStatus );
  add_theme_simple_icon_column( s, i18n( "Spam/Ham" ), QLatin1String( "mail-mark-junk" ), Theme::ContentItem::SpamHamStateIcon, false, SortOrder::NoMessageSorting );
  add_theme_simple_icon_column( s, i18n( "Watched/Ignored" ), QLatin1String( "mail-thread-watch" ), Theme::ContentItem::WatchedIgnoredStateIcon, false, SortOrder::NoMessageSorting );
  add_theme_simple_icon_column( s, i18n( "Encryption" ), QLatin1String( "mail-encrypted-full" ), Theme::ContentItem::EncryptionStateIcon, false, SortOrder::NoMessageSorting );
  add_theme_simple_icon_column( s, i18n( "Signature" ), QLatin1String( "mail-signed-verified" ), Theme::ContentItem::SignatureStateIcon, false, SortOrder::NoMessageSorting );
  add_theme_simple_icon_column( s, i18n( "Tag List" ), QLatin1String( "feed-subscribe" ), Theme::ContentItem::TagList, false, SortOrder::NoMessageSorting );

  s->resetColumnState(); // so it's initially set from defaults

  addTheme( s );

  // The Fancy theme

  s = new Theme(
      i18n( "Smart" ),
      i18n( "A smart multiline and multi item theme" ), true /*readOnly*/
    );

    c = new Theme::Column();
    c->setLabel( i18n( "Message" ) );

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::ExpandedStateIcon );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::GroupHeaderLabel );
        QFont aBigFont = KGlobalSettings::generalFont();
        aBigFont.setBold( true );
        i->setFont( aBigFont );
        i->setUseCustomFont( true );
      r->addLeftItem( i );
    c->addGroupHeaderRow( r );

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::Subject );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::ReadStateIcon );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::RepliedStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::AttachmentStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::AnnotationIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::InvitationIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::EncryptionStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::SignatureStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::TagList );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
    c->addMessageRow( r );

  Theme::Row * firstFancyRow = r; // save it so we can continue adding stuff below (after cloning the theme)

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::SenderOrReceiver );
        i->setSoftenByBlending( true );
        QFont aItalicFont = KGlobalSettings::generalFont();
        aItalicFont.setItalic( true );
        i->setFont( aItalicFont );
        i->setUseCustomFont( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::Date );
        i->setSoftenByBlending( true );
        i->setFont( aItalicFont );
        i->setUseCustomFont( true );
      r->addRightItem( i );
    c->addMessageRow( r );

  s->addColumn( c );

  // clone the "Fancy theme" here so we'll use it as starting point for the "Fancy with clickable status"
  Theme * fancyWithClickableStatus = new Theme( *s );
  fancyWithClickableStatus->detach();
  fancyWithClickableStatus->generateUniqueId();

  // and continue the "Fancy" specific settings
  r = firstFancyRow;

        i = new Theme::ContentItem( Theme::ContentItem::ActionItemStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::ImportantStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::SpamHamStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::WatchedIgnoredStateIcon );
        i->setHideWhenDisabled( true );
      r->addRightItem( i );

  s->setViewHeaderPolicy( Theme::NeverShowHeader );

  s->resetColumnState(); // so it's initially set from defaults

  addTheme( s );


  // The "Fancy with Clickable Status" theme

  s = fancyWithClickableStatus;

  s->setName( i18n( "Smart with Clickable Status" ) );
  s->setDescription( i18n( "A smart multiline and multi item theme with a clickable status column" ) );
  s->setReadOnly( true );

    c = new Theme::Column();
    c->setLabel( i18n( "Status" ) );
    c->setVisibleByDefault( true );

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::ActionItemStateIcon );
        i->setSoftenByBlendingWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::ImportantStateIcon );
        i->setSoftenByBlendingWhenDisabled( true );
      r->addLeftItem( i );
    c->addMessageRow( r );

      r = new Theme::Row();
        i = new Theme::ContentItem( Theme::ContentItem::SpamHamStateIcon );
        i->setSoftenByBlendingWhenDisabled( true );
      r->addLeftItem( i );
        i = new Theme::ContentItem( Theme::ContentItem::WatchedIgnoredStateIcon );
        i->setSoftenByBlendingWhenDisabled( true );
      r->addLeftItem( i );
    c->addMessageRow( r );

  s->addColumn( c );

  s->resetColumnState(); // so it's initially set from defaults

  addTheme( s );
}

void Manager::removeAllThemes()
{
  QHash< QString, Theme * >::ConstIterator end( mThemes.constEnd() );
  for( QHash< QString, Theme * >::ConstIterator it = mThemes.constBegin(); it != end; ++it )
    delete ( *it );

  mThemes.clear();
}

void Manager::themesConfigurationCompleted()
{
  if ( mThemes.isEmpty() )
    createDefaultThemes(); // panic

  saveConfiguration(); // just to be sure :)

  // notify all the widgets that they should reload the option set combos
  emit themesChanged();
}

void Manager::reloadAllWidgets()
{

  QList< Widget * >::ConstIterator end( mWidgetList.constEnd() );
  for( QList< Widget * >::ConstIterator it = mWidgetList.constBegin(); it != end; ++it )
  {
    if ( ( *it )->view() )
      ( *it )->view()->reload();
  }
}


void Manager::reloadGlobalConfiguration()
{
  // This is called when configuration changes (probably edited by the options dialog)
  const int oldDateFormat = (int)mDateFormatter->format();
  const QString oldDateCustomFormat = mDateFormatter->customFormat();

  loadGlobalConfiguration();

  if (
       ( oldDateFormat != (int)mDateFormatter->format() ) ||
       ( oldDateCustomFormat != mDateFormatter->customFormat() )
     )
    reloadAllWidgets();
}


void Manager::loadGlobalConfiguration()
{
  // Load the date format
  const KMime::DateFormatter::FormatType type = static_cast<KMime::DateFormatter::FormatType>(
       MessageCore::GlobalSettings::self()->dateFormat() );
  mDateFormatter->setCustomFormat( MessageCore::GlobalSettings::self()->customDateFormat() );
  mDateFormatter->setFormat( type );
}

void Manager::loadConfiguration()
{
  loadGlobalConfiguration();

  {
    // load Aggregations

    KConfigGroup conf( Settings::self()->config(), "MessageListView::Aggregations" );

    mAggregations.clear();

    const int cnt = conf.readEntry( "Count", 0 );

    int idx = 0;
    while ( idx < cnt )
    {
      const QString data = conf.readEntry( QString::fromLatin1( "Set%1" ).arg( idx ), QString() );
      if ( !data.isEmpty() )
      {
        Aggregation * set = new Aggregation();
        if ( set->loadFromString( data ) )
        {
          if ( Aggregation * old = mAggregations.value( set->id() ) )
            delete old;
          mAggregations.insert( set->id(), set );
        } else {
          delete set; // b0rken
        }
      }
      idx++;
    }

    if ( mAggregations.isEmpty() )
    {
      // don't allow zero configuration, create some presets
      createDefaultAggregations();
    }
  }

  {
    // load Themes

    KConfigGroup conf( Settings::self()->config(), "MessageListView::Themes" );

    mThemes.clear();

    const int cnt = conf.readEntry( "Count", 0 );

    int idx = 0;
    while ( idx < cnt )
    {
      const QString data = conf.readEntry( QString::fromLatin1( "Set%1" ).arg( idx ), QString() );
      if ( !data.isEmpty() )
      {
        Theme * set = new Theme();
        if ( set->loadFromString( data ) )
        {
          if ( Theme * old = mThemes.value( set->id() ) )
            delete old;
          mThemes.insert( set->id(), set );
        } else {
          kWarning() << "Saved theme loading failed";
          delete set; // b0rken
        }
      }
      ++idx;
    }

    if ( mThemes.isEmpty() )
    {
      // don't allow zero configuration, create some presets
      createDefaultThemes();
    }
  }

}

void Manager::saveGlobalConfiguration()
{
  Settings::self()->writeConfig();
}

void Manager::saveConfiguration()
{
  saveGlobalConfiguration();

  {
    // store aggregations

    KConfigGroup conf( Settings::self()->config(), "MessageListView::Aggregations" );
    //conf.clear();

    conf.writeEntry( "Count", mAggregations.count() );

    int idx = 0;
    QHash< QString, Aggregation * >::ConstIterator end( mAggregations.end() );
    for( QHash< QString, Aggregation * >::ConstIterator it = mAggregations.constBegin(); it != end; ++it )
    {
      conf.writeEntry( QString::fromLatin1( "Set%1" ).arg( idx ), ( *it )->saveToString() );
      ++idx;
    }
  }

  {
    // store themes

    KConfigGroup conf( Settings::self()->config(), "MessageListView::Themes" );
    //conf.clear();

    conf.writeEntry( "Count", mThemes.count() );

    int idx = 0;
    QHash< QString, Theme * >::ConstIterator end( mThemes.constEnd() );
    for( QHash< QString, Theme * >::ConstIterator it = mThemes.constBegin(); it != end; ++it )
    {
      conf.writeEntry( QString::fromLatin1( "Set%1" ).arg( idx ), ( *it )->saveToString() );
      ++idx;
    }
  }

  Settings::self()->config()->sync();
}


