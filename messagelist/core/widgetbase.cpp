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

#include "core/widgetbase.h"

#include "core/aggregation.h"
#include "core/theme.h"
#include "core/filter.h"
#include "core/manager.h"
#include "core/optionset.h"
#include "core/view.h"
#include "core/model.h"
#include "core/messageitem.h"
#include "core/storagemodelbase.h"
#include "core/settings.h"

#include "utils/configureaggregationsdialog.h"
#include "utils/configurethemesdialog.h"

#include <QActionGroup>
#include <QBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

#include <KDE/KAction>
#include <KDE/KComboBox>
#include <KDE/KConfig>
#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KIconLoader>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KMenu>

#include <akonadi/kmime/messagestatus.h>

using namespace MessageList::Core;

class Widget::Private
{
public:
  Private( Widget *owner )
    : q( owner ), mView( 0 ), mSearchEdit( 0 ),
      mSearchTimer( 0 ), mStatusFilterCombo( 0 ),
      mOpenFullSearchButton( 0 ),
      mStorageModel( 0 ), mAggregation( 0 ),
      mTheme( 0 ), mFilter( 0 ),
      mStorageUsesPrivateTheme( false ),
      mStorageUsesPrivateAggregation( false ),
      mStorageUsesPrivateSortOrder( false ),
      mFirstTagInComboIndex( -1 ) { }


  /**
   * Small helper for switching SortOrder::MessageSorting and SortOrder::SortDirection
   * on the fly.
   * After doing this, the sort indicator in the header is updated.
   */
  void switchMessageSorting( SortOrder::MessageSorting messageSorting,
                             SortOrder::SortDirection sortDirection,
                             int logicalHeaderColumnIndex );

  /**
   * Check if our sort order can still be used with this aggregation.
   * This can happen if the global aggregation changed, for example we can now
   * have "most recent in subtree" sorting with an aggregation without threading.
   * If this happens, reset to the default sort order and don't use the global sort
   * order.
   */
  void checkSortOrder( const StorageModel *storageModel );

  void setDefaultAggregationForStorageModel( const StorageModel * storageModel );
  void setDefaultThemeForStorageModel( const StorageModel * storageModel );
  void setDefaultSortOrderForStorageModel( const StorageModel * storageModel );
  void applyFilter();

  Widget * const q;

  View *mView;
  QString mLastAggregationId;
  QString mLastThemeId;
  KLineEdit *mSearchEdit;
  QTimer *mSearchTimer;
  KComboBox *mStatusFilterCombo;
  QToolButton *mOpenFullSearchButton;

  StorageModel * mStorageModel;          ///< The currently displayed storage. The storage itself
                                         ///  is owned by MessageList::Widget.
  Aggregation * mAggregation;            ///< The currently set aggregation mode, a deep copy
  Theme * mTheme;                        ///< The currently set theme, a deep copy
  SortOrder mSortOrder;                  ///< The currently set sort order
  Filter * mFilter;                      ///< The currently applied filter, owned by us.
  bool mStorageUsesPrivateTheme;         ///< true if the current folder does not use the global theme
  bool mStorageUsesPrivateAggregation;   ///< true if the current folder does not use the global aggregation
  bool mStorageUsesPrivateSortOrder;     ///< true if the current folder does not use the global sort order
  int mFirstTagInComboIndex;             ///< the index of the combobox where the first tag starts

};

Widget::Widget( QWidget *pParent )
  : QWidget( pParent ), d( new Private( this ) )
{
  Manager::registerWidget( this );
  connect( Manager::instance(), SIGNAL(aggregationsChanged()),
           this, SLOT(aggregationsChanged() ) );
  connect( Manager::instance(), SIGNAL(themesChanged()),
           this, SLOT(themesChanged() ) );

  setAutoFillBackground( true );
  setObjectName( QLatin1String( "messagelistwidget" ) );

  QGridLayout * g = new QGridLayout( this );
  g->setMargin( 2 ); // use a smaller default
  g->setSpacing( 2 );

  d->mSearchEdit = new KLineEdit( this );
  d->mSearchEdit->setClickMessage( i18nc( "Search for messages.", "Search" ) );
  d->mSearchEdit->setObjectName( QLatin1String( "quicksearch" ) );
  d->mSearchEdit->setClearButtonShown( true );
  d->mSearchEdit->setVisible( Settings::self()->showQuickSearch() );

  connect( d->mSearchEdit, SIGNAL( textEdited( const QString & ) ),
           SLOT( searchEditTextEdited( const QString & ) ) );

  connect( d->mSearchEdit, SIGNAL( clearButtonClicked() ),
           SLOT( searchEditClearButtonClicked() ) );

  g->addWidget( d->mSearchEdit, 0, 0 );

  // The status filter button. Will be populated later, as populateStatusFilterCombo() is virtual
  d->mStatusFilterCombo = new KComboBox( this ) ;
  d->mStatusFilterCombo->setVisible( Settings::self()->showQuickSearch() );
  g->addWidget( d->mStatusFilterCombo, 0, 1 );

  // The "Open Full Search" button
  d->mOpenFullSearchButton = new QToolButton( this );
  d->mOpenFullSearchButton->setIcon( KIcon( QLatin1String( "edit-find-mail" ) ) );
  d->mOpenFullSearchButton->setText( i18n( "Open Full Search" ) );
  d->mOpenFullSearchButton->setToolTip( d->mOpenFullSearchButton->text() );
  d->mOpenFullSearchButton->setVisible( Settings::self()->showQuickSearch() );
  g->addWidget( d->mOpenFullSearchButton, 0, 2 );

  connect( d->mOpenFullSearchButton, SIGNAL( clicked() ),
           this, SIGNAL( fullSearchRequest() ) );


  d->mView = new View( this );
  d->mView->setSortOrder( &d->mSortOrder );
  d->mView->setObjectName( QLatin1String( "messagealistview" ) );
  g->addWidget( d->mView, 1, 0, 1, 6 );

  connect( d->mView->header(), SIGNAL( sectionClicked( int ) ),
           SLOT( slotViewHeaderSectionClicked( int ) ) );

  g->setRowStretch( 1, 1 );
  g->setColumnStretch( 0, 1 );

  d->mSearchEdit->setEnabled( false );
  d->mStatusFilterCombo->setEnabled( false );

  d->mSearchTimer = 0;
}

Widget::~Widget()
{
  d->mView->setStorageModel( 0 );

  Manager::unregisterWidget( this );

  delete d->mSearchTimer;
  delete d->mTheme;
  delete d->mAggregation;
  delete d->mFilter;
  delete d->mStorageModel;

  delete d;
}

void Widget::changeQuicksearchVisibility()
{
  KLineEdit * const lineEdit = d->mSearchEdit;
  QWidget * const comboBox = d->mStatusFilterCombo;
  QWidget * const fullSearchButton = d->mOpenFullSearchButton;
  if ( lineEdit ) {
    const bool visible = lineEdit->isVisible() &&
                         comboBox->isVisible() &&
                         fullSearchButton->isVisible();
    if ( visible ) {
      //if we hide it we do not want to apply the filter,
      //otherwise someone is maybe stuck with x new emails
      //and cannot read it because of filter
      lineEdit->clear();

      //we focus the message list if we hide the searchbar
      d->mView->setFocus( Qt::OtherFocusReason );
    }
    else {
      // on show: we focus the lineedit for fast filtering
      lineEdit->setFocus( Qt::OtherFocusReason );
    }
    lineEdit->setVisible( !visible );
    comboBox->setVisible( !visible );
    fullSearchButton->setVisible( !visible );
    Settings::self()->setShowQuickSearch( !visible );
  }
}

void Widget::populateStatusFilterCombo()
{
  d->mStatusFilterCombo->clear();

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "system-run" )), i18n( "Any Status" ), 0 );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-unread" )),
                                  i18nc( "@action:inmenu Status of a message", "Unread" ),
                                  Akonadi::MessageStatus::statusUnread().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-replied" )),
                                  i18nc( "@action:inmenu Status of a message", "Replied" ),
                                  Akonadi::MessageStatus::statusReplied().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-forwarded" )),
                                  i18nc( "@action:inmenu Status of a message", "Forwarded" ),
                                  Akonadi::MessageStatus::statusForwarded().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "emblem-important" )),
                                  i18nc( "@action:inmenu Status of a message", "Important"),
                                  Akonadi::MessageStatus::statusImportant().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-task" )),
                                  i18nc( "@action:inmenu Status of a message", "Action Item" ),
                                  Akonadi::MessageStatus::statusToAct().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-thread-watch" )),
                                  i18nc( "@action:inmenu Status of a message", "Watched" ),
                                  Akonadi::MessageStatus::statusWatched().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-thread-ignored" )),
                                  i18nc( "@action:inmenu Status of a message", "Ignored" ),
                                  Akonadi::MessageStatus::statusIgnored().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-attachment" )),
                                  i18nc( "@action:inmenu Status of a message", "Has Attachment" ),
                                  Akonadi::MessageStatus::statusHasAttachment().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-invitation" )),
                                  i18nc( "@action:inmenu Status of a message", "Has Invitation" ),
                                  Akonadi::MessageStatus::statusHasInvitation().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-mark-junk" )),
                                  i18nc( "@action:inmenu Status of a message", "Spam" ),
                                  Akonadi::MessageStatus::statusSpam().toQInt32() );

  d->mStatusFilterCombo->addItem( SmallIcon(QLatin1String( "mail-mark-notjunk" )),
                                  i18nc( "@action:inmenu Status of a message", "Ham" ),
                                  Akonadi::MessageStatus::statusHam().toQInt32() );

  d->mFirstTagInComboIndex = d->mStatusFilterCombo->count();
  fillMessageTagCombo( d->mStatusFilterCombo );

  disconnect( d->mStatusFilterCombo, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( statusSelected( int ) ) );
  connect( d->mStatusFilterCombo, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( statusSelected( int ) ) );
}

MessageItem *Widget::currentMessageItem() const
{
  return view()->currentMessageItem();
}

Akonadi::MessageStatus Widget::currentFilterStatus() const
{
  if ( !d->mFilter )
    return Akonadi::MessageStatus();

  Akonadi::MessageStatus ret;
  ret.fromQInt32( d->mFilter->statusMask() );
  return ret;
}

QString Widget::currentFilterSearchString() const
{
  if ( !d->mFilter )
    return QString();

  return d->mFilter->searchString();
}

QString Widget::currentFilterTagId() const
{
  if ( !d->mFilter )
    return QString();

  return d->mFilter->tagId();
}

void Widget::Private::setDefaultAggregationForStorageModel( const StorageModel * storageModel )
{
  const Aggregation * opt = Manager::instance()->aggregationForStorageModel( storageModel, &mStorageUsesPrivateAggregation );

  Q_ASSERT( opt );

  delete mAggregation;
  mAggregation = new Aggregation( *opt );

  mView->setAggregation( mAggregation );

  mLastAggregationId = opt->id();
}

void Widget::Private::setDefaultThemeForStorageModel( const StorageModel * storageModel )
{
  const Theme * opt = Manager::instance()->themeForStorageModel( storageModel, &mStorageUsesPrivateTheme );

  Q_ASSERT( opt );

  delete mTheme;
  mTheme = new Theme( *opt );

  mView->setTheme( mTheme );

  mLastThemeId = opt->id();
}

void Widget::Private::checkSortOrder( const StorageModel *storageModel )
{
  if ( storageModel && mAggregation && !mSortOrder.validForAggregation( mAggregation ) ) {
    kDebug() << "Could not restore sort order for folder" << storageModel->id();
    mSortOrder = SortOrder::defaultForAggregation( mAggregation, mSortOrder );

    // Change the global sort order if the sort order didn't fit the global aggregation.
    // Otherwise, if it is a per-folder aggregation, make the sort order per-folder too.
    if ( mStorageUsesPrivateAggregation )
      mStorageUsesPrivateSortOrder = true;
    if ( mStorageModel ) {
      Manager::instance()->saveSortOrderForStorageModel( storageModel, mSortOrder,
                                                         mStorageUsesPrivateSortOrder );
    }
    switchMessageSorting( mSortOrder.messageSorting(), mSortOrder.messageSortDirection(), -1 );
  }

}

void Widget::Private::setDefaultSortOrderForStorageModel( const StorageModel * storageModel )
{
  // Load the sort order from config and update column headers
  mSortOrder = Manager::instance()->sortOrderForStorageModel( storageModel, &mStorageUsesPrivateSortOrder );
  switchMessageSorting( mSortOrder.messageSorting(), mSortOrder.messageSortDirection(), -1 );
  checkSortOrder( storageModel );
}

void Widget::setStorageModel( StorageModel * storageModel, PreSelectionMode preSelectionMode )
{
  if ( storageModel == d->mStorageModel )
    return; // nuthin to do here

  if ( d->mStorageModel )
  {
    // Save the current selection
    MessageItem * lastSelectedMessageItem = d->mView->currentMessageItem( false );
    Manager::instance()->savePreSelectedMessageForStorageModel(
        d->mStorageModel,
        lastSelectedMessageItem ? lastSelectedMessageItem->uniqueId() : 0
      );
  }

  d->setDefaultAggregationForStorageModel( storageModel );
  d->setDefaultThemeForStorageModel( storageModel );
  d->setDefaultSortOrderForStorageModel( storageModel );

  if ( d->mSearchTimer )
  {
    d->mSearchTimer->stop();
    delete d->mSearchTimer;
    d->mSearchTimer = 0;
  }

  d->mSearchEdit->setText( QString() );

  if ( d->mFilter ) {
    resetFilter();
  }

  StorageModel * oldModel = d->mStorageModel;

  d->mStorageModel = storageModel;

  d->mView->setStorageModel( d->mStorageModel, preSelectionMode );

  delete oldModel;

  d->mStatusFilterCombo->setEnabled( d->mStorageModel );
  d->mSearchEdit->setEnabled( d->mStorageModel );
}

StorageModel *Widget::storageModel() const
{
  return d->mStorageModel;
}

KLineEdit *Widget::quickSearch() const
{
  return d->mSearchEdit;
}

View *Widget::view() const
{
  return d->mView;
}

void Widget::themeMenuAboutToShow()
{
  if ( !d->mStorageModel )
    return;

  KMenu * menu = dynamic_cast< KMenu * >( sender() );
  if ( !menu )
    return;

  menu->clear();

  menu->addTitle( i18n( "Theme" ) );

  QActionGroup * grp = new QActionGroup( menu );

  const QHash< QString, Theme * > & themes = Manager::instance()->themes();

  QAction * act;

  QList< const Theme * > sortedThemes;

  for ( QHash< QString, Theme * >::ConstIterator ci = themes.constBegin(); ci != themes.constEnd(); ++ci )
  {
    int idx = 0;
    int cnt = sortedThemes.count();
    while ( idx < cnt )
    {
      if ( sortedThemes.at( idx )->name() > ( *ci )->name() )
      {
        sortedThemes.insert( idx, *ci );
        break;
      }
      idx++;
    }

    if ( idx == cnt )
      sortedThemes.append( *ci );
  }

  for ( QList< const Theme * >::ConstIterator it = sortedThemes.constBegin(); it != sortedThemes.constEnd(); ++it )
  {
    act = menu->addAction( ( *it )->name() );
    act->setCheckable( true );
    grp->addAction( act );
    act->setChecked( d->mLastThemeId == ( *it )->id() );
    act->setData( QVariant( ( *it )->id() ) );
    connect( act, SIGNAL( triggered( bool ) ),
             SLOT( themeSelected( bool ) ) );
  }

  menu->addSeparator();

  act = menu->addAction( i18n( "Configure..." ) );
  connect( act, SIGNAL( triggered( bool ) ),
           SLOT( configureThemes() ) );
}

void Widget::setPrivateSortOrderForStorage()
{
  if ( !d->mStorageModel )
    return;

  d->mStorageUsesPrivateSortOrder = !d->mStorageUsesPrivateSortOrder;

  Manager::instance()->saveSortOrderForStorageModel( d->mStorageModel, d->mSortOrder,
                                                     d->mStorageUsesPrivateSortOrder );
}

void Widget::configureThemes()
{
  Utils::ConfigureThemesDialog *dialog = new Utils::ConfigureThemesDialog( window() );
  dialog->selectTheme( d->mLastThemeId );
  dialog->show();
}

void Widget::themeSelected( bool )
{
  if ( !d->mStorageModel )
    return; // nuthin to do

  QAction * act = dynamic_cast< QAction * >( sender() );
  if ( !act )
    return;

  QVariant v = act->data();
  QString id = v.toString();

  if ( id.isEmpty() )
    return;

  const Theme * opt = Manager::instance()->theme( id );

  delete d->mTheme;
  d->mTheme = new Theme( *opt );

  d->mView->setTheme( d->mTheme );

  d->mLastThemeId = opt->id();

  //mStorageUsesPrivateTheme = false;

  Manager::instance()->saveThemeForStorageModel( d->mStorageModel, opt->id(), d->mStorageUsesPrivateTheme );

  d->mView->reload();

}

void Widget::aggregationMenuAboutToShow()
{
  KMenu * menu = dynamic_cast< KMenu * >( sender() );
  if ( !menu )
    return;

  menu->clear();

  menu->addTitle( i18n( "Aggregation" ) );

  QActionGroup * grp = new QActionGroup( menu );

  const QHash< QString, Aggregation * > & aggregations = Manager::instance()->aggregations();

  QAction * act;

  QList< const Aggregation * > sortedAggregations;

  for ( QHash< QString, Aggregation * >::ConstIterator ci = aggregations.constBegin(); ci != aggregations.constEnd(); ++ci )
  {
    int idx = 0;
    int cnt = sortedAggregations.count();
    while ( idx < cnt )
    {
      if ( sortedAggregations.at( idx )->name() > ( *ci )->name() )
      {
        sortedAggregations.insert( idx, *ci );
        break;
      }
      idx++;
    }

    if ( idx == cnt )
      sortedAggregations.append( *ci );
  }

  for ( QList< const Aggregation * >::ConstIterator it = sortedAggregations.constBegin(); it != sortedAggregations.constEnd(); ++it )
  {
    act = menu->addAction( ( *it )->name() );
    act->setCheckable( true );
    grp->addAction( act );
    act->setChecked( d->mLastAggregationId == ( *it )->id() );
    act->setData( QVariant( ( *it )->id() ) );
    connect( act, SIGNAL( triggered( bool ) ),
             SLOT( aggregationSelected( bool ) ) );
  }

  menu->addSeparator();

  act = menu->addAction( i18n( "Configure..." ) );
  act->setData( QVariant( QString() ) );
  connect( act, SIGNAL( triggered( bool ) ),
           SLOT( aggregationSelected( bool ) ) );
}

void Widget::aggregationSelected( bool )
{
  QAction * act = dynamic_cast< QAction * >( sender() );
  if ( !act )
    return;

  QVariant v = act->data();
  QString id = v.toString();

  if ( id.isEmpty() ) {
    Utils::ConfigureAggregationsDialog *dialog = new Utils::ConfigureAggregationsDialog( window() );
    dialog->selectAggregation( d->mLastAggregationId );
    dialog->show();
    return;
  }

  if ( !d->mStorageModel )
    return; // nuthin to do

  const Aggregation * opt = Manager::instance()->aggregation( id );

  delete d->mAggregation;
  d->mAggregation = new Aggregation( *opt );

  d->mView->setAggregation( d->mAggregation );

  d->mLastAggregationId = opt->id();

  //mStorageUsesPrivateAggregation = false;

  Manager::instance()->saveAggregationForStorageModel( d->mStorageModel, opt->id(), d->mStorageUsesPrivateAggregation );

  // The sort order might not be valid anymore for this aggregation
  d->checkSortOrder( d->mStorageModel );

  d->mView->reload();

}

void Widget::sortOrderMenuAboutToShow()
{
  if ( !d->mAggregation )
    return;

  KMenu * menu = dynamic_cast< KMenu * >( sender() );
  if ( !menu )
    return;

  menu->clear();

  menu->addTitle( i18n( "Message Sort Order" ) );

  QActionGroup * grp;
  QAction * act;
  QList< QPair< QString, int > > options;
  QList< QPair< QString, int > >::ConstIterator it;

  grp = new QActionGroup( menu );

  options = SortOrder::enumerateMessageSortingOptions( d->mAggregation->threading() );

  for ( it = options.constBegin(); it != options.constEnd(); ++it )
  {
    act = menu->addAction( ( *it ).first );
    act->setCheckable( true );
    grp->addAction( act );
    act->setChecked( d->mSortOrder.messageSorting() == ( *it ).second );
    act->setData( QVariant( ( *it ).second ) );
  }

  connect( grp, SIGNAL( triggered( QAction * ) ),
           SLOT( messageSortingSelected( QAction * ) ) );

  options = SortOrder::enumerateMessageSortDirectionOptions( d->mSortOrder.messageSorting() );

  if ( options.size() >= 2 )
  {
    menu->addTitle( i18n( "Message Sort Direction" ) );

    grp = new QActionGroup( menu );

    for ( it = options.constBegin(); it != options.constEnd(); ++it )
    {
      act = menu->addAction( ( *it ).first );
      act->setCheckable( true );
      grp->addAction( act );
      act->setChecked( d->mSortOrder.messageSortDirection() == ( *it ).second );
      act->setData( QVariant( ( *it ).second ) );
    }

    connect( grp, SIGNAL( triggered( QAction * ) ),
             SLOT( messageSortDirectionSelected( QAction * ) ) );
  }

  options = SortOrder::enumerateGroupSortingOptions( d->mAggregation->grouping() );

  if ( options.size() >= 2 )
  {
    menu->addTitle( i18n( "Group Sort Order" ) );

    grp = new QActionGroup( menu );

    for ( it = options.constBegin(); it != options.constEnd(); ++it )
    {
      act = menu->addAction( ( *it ).first );
      act->setCheckable( true );
      grp->addAction( act );
      act->setChecked( d->mSortOrder.groupSorting() == ( *it ).second );
      act->setData( QVariant( ( *it ).second ) );
    }

    connect( grp, SIGNAL( triggered( QAction * ) ),
             SLOT( groupSortingSelected( QAction * ) ) );
  }

  options = SortOrder::enumerateGroupSortDirectionOptions( d->mAggregation->grouping(),
                                                           d->mSortOrder.groupSorting() );

  if ( options.size() >= 2 )
  {
    menu->addTitle( i18n( "Group Sort Direction" ) );

    grp = new QActionGroup( menu );

    for ( it = options.constBegin(); it != options.constEnd(); ++it )
    {
      act = menu->addAction( ( *it ).first );
      act->setCheckable( true );
      grp->addAction( act );
      act->setChecked( d->mSortOrder.groupSortDirection() == ( *it ).second );
      act->setData( QVariant( ( *it ).second ) );
    }

    connect( grp, SIGNAL( triggered( QAction * ) ),
             SLOT( groupSortDirectionSelected( QAction * ) ) );
  }

  menu->addSeparator();
  act = menu->addAction( i18n( "Folder Always Uses This Sort Order" ) );
  act->setCheckable( true );
  act->setChecked( d->mStorageUsesPrivateSortOrder );
  connect( act, SIGNAL( triggered( bool ) ),
           SLOT( setPrivateSortOrderForStorage() ) );
}

void Widget::Private::switchMessageSorting( SortOrder::MessageSorting messageSorting,
                                            SortOrder::SortDirection sortDirection,
                                            int logicalHeaderColumnIndex )
{
  mSortOrder.setMessageSorting( messageSorting );
  mSortOrder.setMessageSortDirection( sortDirection );

  // If the logicalHeaderColumnIndex was specified then we already know which
  // column we should set the sort indicator to. If it wasn't specified (it's -1)
  // then we need to find it out in the theme.

  if ( logicalHeaderColumnIndex == -1 )
  {
    // try to find the specified message sorting in the theme columns
    const QList< Theme::Column * > & columns = mTheme->columns();
    int idx = 0;

    // First try with a well defined message sorting.

    foreach( const Theme::Column* column, columns )
    {
      if ( !mView->header()->isSectionHidden( idx ) )
      {
        if ( column->messageSorting() == messageSorting )
        {
          // found a visible column with this message sorting
          logicalHeaderColumnIndex = idx;
          break;
        }
      }
      ++idx;
    }

    // if still not found, try again with a wider range
    if ( logicalHeaderColumnIndex == 1 )
    {
      idx = 0;
      foreach( const Theme::Column* column, columns )
      {
        if ( !mView->header()->isSectionHidden( idx ) )
        {
          if (
               (
                 ( column->messageSorting() == SortOrder::SortMessagesBySenderOrReceiver ) ||
                 ( column->messageSorting() == SortOrder::SortMessagesByReceiver ) ||
                 ( column->messageSorting() == SortOrder::SortMessagesBySender )
               ) &&
               (
                 ( messageSorting == SortOrder::SortMessagesBySenderOrReceiver ) ||
                 ( messageSorting == SortOrder::SortMessagesByReceiver ) ||
                 ( messageSorting == SortOrder::SortMessagesBySender )
               )
             )
          {
            // found a visible column with this message sorting
            logicalHeaderColumnIndex = idx;
            break;
          }
        }
        ++idx;
      }
    }
  }

  if ( logicalHeaderColumnIndex == -1 )
  {
    // not found: either not a column-based sorting or the related column is hidden
    mView->header()->setSortIndicatorShown( false );
    return;
  }

  mView->header()->setSortIndicatorShown( true );

  if ( sortDirection == SortOrder::Ascending )
    mView->header()->setSortIndicator( logicalHeaderColumnIndex, Qt::AscendingOrder );
  else
    mView->header()->setSortIndicator( logicalHeaderColumnIndex, Qt::DescendingOrder );
}

void Widget::messageSortingSelected( QAction *action )
{
  if ( !d->mAggregation )
    return;
  if ( !action )
    return;

  bool ok;
  SortOrder::MessageSorting ord = static_cast< SortOrder::MessageSorting >( action->data().toInt( &ok ) );

  if ( !ok )
    return;

  d->switchMessageSorting( ord, d->mSortOrder.messageSortDirection(), -1 );
  Manager::instance()->saveSortOrderForStorageModel( d->mStorageModel, d->mSortOrder,
                                                     d->mStorageUsesPrivateSortOrder );

  d->mView->reload();

}

void Widget::messageSortDirectionSelected( QAction *action )
{
  if ( !d->mAggregation )
    return;
  if ( !action )
    return;

  bool ok;
  SortOrder::SortDirection ord = static_cast< SortOrder::SortDirection >( action->data().toInt( &ok ) );

  if ( !ok )
    return;

  d->switchMessageSorting( d->mSortOrder.messageSorting(), ord, -1 );
  Manager::instance()->saveSortOrderForStorageModel( d->mStorageModel, d->mSortOrder,
                                                     d->mStorageUsesPrivateSortOrder );

  d->mView->reload();

}

void Widget::groupSortingSelected( QAction *action )
{
  if ( !d->mAggregation )
    return;
  if ( !action )
    return;

  bool ok;
  SortOrder::GroupSorting ord = static_cast< SortOrder::GroupSorting >( action->data().toInt( &ok ) );

  if ( !ok )
    return;

  d->mSortOrder.setGroupSorting( ord );
  Manager::instance()->saveSortOrderForStorageModel( d->mStorageModel, d->mSortOrder,
                                                     d->mStorageUsesPrivateSortOrder );

  d->mView->reload();

}

void Widget::groupSortDirectionSelected( QAction *action )
{
  if ( !d->mAggregation )
    return;
  if ( !action )
    return;

  bool ok;
  SortOrder::SortDirection ord = static_cast< SortOrder::SortDirection >( action->data().toInt( &ok ) );

  if ( !ok )
    return;

  d->mSortOrder.setGroupSortDirection( ord );
  Manager::instance()->saveSortOrderForStorageModel( d->mStorageModel, d->mSortOrder,
                                                     d->mStorageUsesPrivateSortOrder );

  d->mView->reload();

}

void Widget::resetFilter()
{
  delete d->mFilter;
  d->mFilter = 0;
  d->mView->model()->setFilter( 0 );
  d->mStatusFilterCombo->setCurrentIndex( 0 );
}

void Widget::slotViewHeaderSectionClicked( int logicalIndex )
{
  if ( !d->mTheme )
    return;

  if ( !d->mAggregation )
    return;

  if ( logicalIndex >= d->mTheme->columns().count() )
    return;

  const Theme::Column * column = d->mTheme->column( logicalIndex );
  if ( !column )
    return; // should never happen...

  if ( column->messageSorting() == SortOrder::NoMessageSorting )
    return; // this is a null op.


  if ( d->mSortOrder.messageSorting() == column->messageSorting() )
  {
    // switch sort direction
    if ( d->mSortOrder.messageSortDirection() == SortOrder::Ascending )
      d->switchMessageSorting( d->mSortOrder.messageSorting(), SortOrder::Descending, logicalIndex );
    else
      d->switchMessageSorting( d->mSortOrder.messageSorting(), SortOrder::Ascending, logicalIndex );
  } else {
    // keep sort direction but switch sort order
    d->switchMessageSorting( column->messageSorting(), d->mSortOrder.messageSortDirection(), logicalIndex );
  }
  Manager::instance()->saveSortOrderForStorageModel( d->mStorageModel, d->mSortOrder,
                                                     d->mStorageUsesPrivateSortOrder );

  d->mView->reload();

}

void Widget::themesChanged()
{
  d->setDefaultThemeForStorageModel( d->mStorageModel );

  d->mView->reload();
}

void Widget::aggregationsChanged()
{
  d->setDefaultAggregationForStorageModel( d->mStorageModel );
  d->checkSortOrder( d->mStorageModel );

  d->mView->reload();
}

void Widget::fillMessageTagCombo( KComboBox* /*combo*/ )
{
  // nothing here: must be overridden in derived classes
}

void Widget::tagIdSelected( QVariant data )
{
  QString tagId = data.toString();

  // Here we arbitrairly set the status to 0, though we *could* allow filtering
  // by status AND tag...

  if ( d->mFilter )
    d->mFilter->setStatusMask( 0 );

  if ( tagId.isEmpty() )
  {
    if ( d->mFilter )
    {
      if ( d->mFilter->isEmpty() ) {
        resetFilter();
        return;
      }
    }
  } else {
    if ( !d->mFilter )
      d->mFilter = new Filter();
    d->mFilter->setTagId( tagId );
  }

  d->mView->model()->setFilter( d->mFilter );
}

void Widget::statusSelected( int index )
{
  if ( index >= d->mFirstTagInComboIndex ) {
    tagIdSelected( d->mStatusFilterCombo->itemData( index ) );
    return;
  }

  bool ok;
  qint32 additionalStatusMask = static_cast< qint32 >( d->mStatusFilterCombo->itemData( index ).toInt( &ok ) );
  if ( !ok )
    return;

  // Here we override the whole status at once.
  // This is a restriction but at least a couple of people
  // are telling me that this way it's more usable. Two are more than me
  // so here we go :)
  qint32 statusMask = 0; //mFilter ? mFilter->statusMask() : 0; <-- this would "or" with the existing mask instead

  // We also arbitrairly set tagId to an empty string, though we *could* allow filtering
  // by status AND tag...
  if ( d->mFilter )
    d->mFilter->setTagId( QString() );

  if ( additionalStatusMask == 0)
  {
    if ( d->mFilter )
    {
      d->mFilter->setStatusMask( 0 );
      if ( d->mFilter->isEmpty() ) {
        resetFilter();
        return;
      }
    }
  } else {
    if ( statusMask & additionalStatusMask )
    {
      // already have this status bit (this actually never happens because of the override above)
      if ( d->mFilter )
      {
        d->mFilter->setStatusMask( statusMask & ~additionalStatusMask );
        if ( d->mFilter->isEmpty() ) {
          resetFilter();
          return;
        }
      } // else nothing to remove (but something weird happened in the code above...)
    } else {
      // don't have this status bit
      if ( !d->mFilter )
        d->mFilter = new Filter();
      d->mFilter->setStatusMask( statusMask | additionalStatusMask );
    }
  }

  d->mView->model()->setFilter( d->mFilter );
}

void Widget::searchEditTextEdited( const QString & )
{
  // This slot is called whenever the user edits the search QLineEdit.
  // Since the user is likely to type more than one character
  // so we start the real search after a short delay in order to catch
  // multiple textEdited() signals.

  if ( !d->mSearchTimer )
  {
    d->mSearchTimer = new QTimer( this );
    connect( d->mSearchTimer, SIGNAL( timeout() ),
             SLOT( searchTimerFired() ) );
  } else {
    d->mSearchTimer->stop(); // eventually
  }

  d->mSearchTimer->setSingleShot( true );
  d->mSearchTimer->start( 1000 );

}

void Widget::searchTimerFired()
{
  // A search is pending.

  if ( d->mSearchTimer )
    d->mSearchTimer->stop();

  if ( !d->mFilter )
    d->mFilter = new Filter();

  QString text = d->mSearchEdit->text();

  d->mFilter->setSearchString( text );
  if ( d->mFilter->isEmpty() ) {
    resetFilter();
    return;
  }

  d->mView->model()->setFilter( d->mFilter );
}

void Widget::searchEditClearButtonClicked()
{
  if ( !d->mFilter )
    return;

  resetFilter();

  d->mView->scrollTo( d->mView->currentIndex(), QAbstractItemView::PositionAtCenter );
}

void Widget::viewMessageSelected( MessageItem * )
{
}

void Widget::viewMessageActivated( MessageItem * )
{
}

void Widget::viewSelectionChanged()
{
}

void Widget::viewMessageListContextPopupRequest( const QList< MessageItem * > &, const QPoint & )
{
}

void Widget::viewGroupHeaderContextPopupRequest( GroupHeaderItem *, const QPoint & )
{
}

void Widget::viewDragEnterEvent( QDragEnterEvent * )
{
}

void Widget::viewDragMoveEvent( QDragMoveEvent * )
{
}

void Widget::viewDropEvent( QDropEvent * )
{
}

void Widget::viewStartDragRequest()
{
}

void Widget::viewJobBatchStarted()
{
}

void Widget::viewJobBatchTerminated()
{
}

void Widget::viewMessageStatusChangeRequest( MessageItem *msg, const Akonadi::MessageStatus &set, const Akonadi::MessageStatus &clear )
{
  Q_UNUSED( msg );
  Q_UNUSED( set );
  Q_UNUSED( clear );
}

void Widget::focusQuickSearch()
{
  if ( d->mSearchEdit )
  {
    d->mSearchEdit->setFocus();
  }
}

bool Widget::isThreaded() const
{
  return d->mView->isThreaded();
}

bool Widget::selectionEmpty() const
{
  return d->mView->selectionEmpty();
}

