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

#include "utils/configureaggregationsdialog.h"
#include "utils/configureaggregationsdialog_p.h"

#include "utils/aggregationeditor.h"
#include "core/aggregation.h"

#include "core/manager.h"

#include <QGridLayout>
#include <QPushButton>
#include <QFrame>
#include <QHash>

#include <KLocale>
#include <KIconLoader>
#include <KMessageBox>

namespace MessageList
{

namespace Utils
{

class AggregationListWidgetItem : public QListWidgetItem
{
private:
  Core::Aggregation * mAggregation;

public:
  AggregationListWidgetItem( QListWidget * par, const Core::Aggregation &set )
    : QListWidgetItem( set.name(), par )
  {
    mAggregation = new Core::Aggregation( set );
  }
  ~AggregationListWidgetItem()
  {
    delete mAggregation;
  }

public:
  Core::Aggregation * aggregation() const
    { return mAggregation; }
  void forgetAggregation()
    { mAggregation = 0; }
};

/**
 * The widget that lists the available Aggregations.
 *
 * At the moment of writing, derived from QListWidget only to override sizeHint().
 */
class AggregationListWidget : public QListWidget
{
public:
  AggregationListWidget( QWidget * parent )
    : QListWidget( parent )
  {
  }
public:

  // need a larger but shorter QListWidget
  QSize sizeHint() const
    { return QSize( 450, 128 ); }
};

} // namespace Utils

} // namespace MessageList

using namespace MessageList::Core;
using namespace MessageList::Utils;

ConfigureAggregationsDialog::ConfigureAggregationsDialog( QWidget *parent )
  : KDialog( parent ), d( new Private( this ) )
{
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowModality( Qt::ApplicationModal ); // FIXME: Sure ?
  setButtons( Ok | Cancel );
  setWindowTitle( i18n( "Customize Message Aggregation Modes" ) );

  QWidget * base = new QWidget( this );
  setMainWidget( base );

  QGridLayout * g = new QGridLayout( base );

  d->mAggregationList = new AggregationListWidget( base );
  d->mAggregationList->setSortingEnabled( true );
  g->addWidget( d->mAggregationList, 0, 0, 7, 1 );

  connect( d->mAggregationList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
           SLOT(aggregationListCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)) );

  d->mNewAggregationButton = new QPushButton( i18n( "New Aggregation" ), base );
  d->mNewAggregationButton->setIcon( KIcon( QLatin1String( "document-new" ) ) );
  d->mNewAggregationButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  g->addWidget( d->mNewAggregationButton, 0, 1 );

  connect( d->mNewAggregationButton, SIGNAL(clicked()),
           SLOT(newAggregationButtonClicked()) );

  d->mCloneAggregationButton = new QPushButton( i18n( "Clone Aggregation" ), base );
  d->mCloneAggregationButton->setIcon( KIcon( QLatin1String( "edit-copy" ) ) );
  d->mCloneAggregationButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  g->addWidget( d->mCloneAggregationButton, 1, 1 );

  connect( d->mCloneAggregationButton, SIGNAL(clicked()),
           SLOT(cloneAggregationButtonClicked()) );

  QFrame * f = new QFrame( base );
  f->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  f->setMinimumHeight( 24 );
  g->addWidget( f, 2, 1, Qt::AlignVCenter );

  d->mExportAggregationButton = new QPushButton( i18n( "Export Aggregation..." ), base );
  g->addWidget( d->mExportAggregationButton, 3, 1 );

  connect( d->mExportAggregationButton, SIGNAL(clicked()),
           SLOT(exportAggregationButtonClicked()) );

  d->mImportAggregationButton = new QPushButton( i18n( "Import Aggregation..." ), base );
  g->addWidget( d->mImportAggregationButton, 4, 1 );
  connect( d->mImportAggregationButton, SIGNAL(clicked()),
           SLOT(importAggregationButtonClicked()) );


  f = new QFrame( base );
  f->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  f->setMinimumHeight( 24 );
  g->addWidget( f, 5, 1, Qt::AlignVCenter );


  d->mDeleteAggregationButton = new QPushButton( i18n( "Delete Aggregation" ), base );
  d->mDeleteAggregationButton->setIcon( KIcon( QLatin1String( "edit-delete" ) ) );
  d->mDeleteAggregationButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  g->addWidget( d->mDeleteAggregationButton, 6, 1 );

  connect( d->mDeleteAggregationButton, SIGNAL(clicked()),
           SLOT(deleteAggregationButtonClicked()) );

  d->mEditor = new AggregationEditor( base );
  g->addWidget( d->mEditor, 8, 0, 1, 2 );

  connect( d->mEditor, SIGNAL(aggregationNameChanged()),
           SLOT(editedAggregationNameChanged()) );

  g->setColumnStretch( 0, 1 );
  g->setRowStretch( 7, 1 );

  connect( this, SIGNAL(okClicked()),
           SLOT(okButtonClicked()) );

  d->fillAggregationList();
}

ConfigureAggregationsDialog::~ConfigureAggregationsDialog()
{
  delete d;
}

void ConfigureAggregationsDialog::selectAggregation( const QString &aggregationId )
{
  AggregationListWidgetItem *item = d->findAggregationItemById( aggregationId );
  if ( item )
    d->mAggregationList->setCurrentItem( item );
}

void ConfigureAggregationsDialog::Private::okButtonClicked()
{
  commitEditor();

  Manager::instance()->removeAllAggregations();

  const int c = mAggregationList->count();
  int i = 0;
  while ( i < c )
  {
    AggregationListWidgetItem * item = dynamic_cast< AggregationListWidgetItem * >( mAggregationList->item( i ) );
    if ( item )
    {
      Manager::instance()->addAggregation( item->aggregation() );
      item->forgetAggregation();
    }
    ++i;
  }

  Manager::instance()->aggregationsConfigurationCompleted();

  q->close(); // this will delete too
}

void ConfigureAggregationsDialog::Private::commitEditor()
{
  Aggregation * editedAggregation = mEditor->editedAggregation();
  if ( !editedAggregation )
    return;

  mEditor->commit();

  AggregationListWidgetItem * editedItem = findAggregationItemByAggregation( editedAggregation );
  if ( editedItem )
    return;
  const QString goodName = uniqueNameForAggregation( editedAggregation->name(), editedAggregation );
  editedAggregation->setName( goodName );
  editedItem->setText( goodName );
}

void ConfigureAggregationsDialog::Private::editedAggregationNameChanged()
{
  Aggregation * set = mEditor->editedAggregation();
  if ( !set )
    return;

  AggregationListWidgetItem * it = findAggregationItemByAggregation( set );
  if ( !it )
    return;

  const QString goodName = uniqueNameForAggregation( set->name(), set );

  it->setText( goodName );
}

void ConfigureAggregationsDialog::Private::fillAggregationList()
{
  const QHash< QString, Aggregation * > & sets = Manager::instance()->aggregations();
  QHash< QString, Aggregation * >::ConstIterator end( sets.constEnd() );
  for( QHash< QString, Aggregation * >::ConstIterator it = sets.constBegin(); it != end; ++it )
    (void)new AggregationListWidgetItem( mAggregationList, *( *it ) );
}

void ConfigureAggregationsDialog::Private::aggregationListCurrentItemChanged( QListWidgetItem * cur, QListWidgetItem * )
{
  commitEditor();

  AggregationListWidgetItem * item = cur ? dynamic_cast< AggregationListWidgetItem * >( cur ) : 0;
  mDeleteAggregationButton->setEnabled( item && !item->aggregation()->readOnly() && ( mAggregationList->count() > 1 ) );

  mCloneAggregationButton->setEnabled( item );
  mEditor->editAggregation( item ? item->aggregation() : 0 );
  if ( item && !item->isSelected() )
    item->setSelected( true ); // make sure it's true
}

AggregationListWidgetItem * ConfigureAggregationsDialog::Private::findAggregationItemByName( const QString &name, Aggregation * skipAggregation )
{
  const int c = mAggregationList->count();
  int i = 0;
  while ( i < c )
  {
    AggregationListWidgetItem * item = dynamic_cast< AggregationListWidgetItem * >( mAggregationList->item( i ) );
    if ( item )
    {
      if ( item->aggregation() != skipAggregation )
      {
        if ( item->aggregation()->name() == name )
          return item;
      }
    }
    i++;
  }
  return 0;
}

AggregationListWidgetItem * ConfigureAggregationsDialog::Private::findAggregationItemById( const QString &aggregationId )
{
  const int c = mAggregationList->count();
  int i = 0;
  while ( i < c )
  {
    AggregationListWidgetItem * item = dynamic_cast< AggregationListWidgetItem * >( mAggregationList->item( i ) );
    if ( item )
    {
      if ( item->aggregation()->id() == aggregationId )
        return item;
    }
    i++;
  }
  return 0;
}

AggregationListWidgetItem * ConfigureAggregationsDialog::Private::findAggregationItemByAggregation( Aggregation * set )
{
  const int c = mAggregationList->count();
  int i = 0;
  while ( i < c )
  {
    AggregationListWidgetItem * item = dynamic_cast< AggregationListWidgetItem * >( mAggregationList->item( i ) );
    if ( item )
    {
      if ( item->aggregation() == set )
        return item;
    }
    i++;
  }
  return 0;
}


QString ConfigureAggregationsDialog::Private::uniqueNameForAggregation( QString baseName, Aggregation * skipAggregation )
{
  QString ret = baseName;
  if( ret.isEmpty() )
    ret = i18n( "Unnamed Aggregation" );

  int idx = 1;

  AggregationListWidgetItem * item = findAggregationItemByName( ret, skipAggregation );
  while ( item )
  {
    idx++;
    ret = QString::fromLatin1( "%1 %2" ).arg( baseName ).arg( idx );
    item = findAggregationItemByName( ret, skipAggregation );
  }
  return ret;
}

void ConfigureAggregationsDialog::Private::newAggregationButtonClicked()
{
  Aggregation emptyAggregation;
  emptyAggregation.setName( uniqueNameForAggregation( i18n( "New Aggregation" ) ) );
  AggregationListWidgetItem * item = new AggregationListWidgetItem( mAggregationList, emptyAggregation );

  mAggregationList->setCurrentItem( item );
  mDeleteAggregationButton->setEnabled( item && !item->aggregation()->readOnly() );
}

void ConfigureAggregationsDialog::Private::cloneAggregationButtonClicked()
{
  AggregationListWidgetItem * item = dynamic_cast< AggregationListWidgetItem * >( mAggregationList->currentItem() );
  if ( !item )
    return;

  Aggregation copyAggregation( *( item->aggregation() ) );
  copyAggregation.setReadOnly( false );
  copyAggregation.generateUniqueId(); // regenerate id so it becomes different
  copyAggregation.setName( uniqueNameForAggregation( item->aggregation()->name() ) );
  item = new AggregationListWidgetItem( mAggregationList, copyAggregation );

  mAggregationList->setCurrentItem( item );
  mDeleteAggregationButton->setEnabled( item && !item->aggregation()->readOnly() );


}

void ConfigureAggregationsDialog::Private::deleteAggregationButtonClicked()
{
  AggregationListWidgetItem * item = dynamic_cast< AggregationListWidgetItem * >( mAggregationList->currentItem() );
  if ( !item )
    return;
  if ( mAggregationList->count() < 2 )
    return; // no way: desperately try to keep at least one option set alive :)

  mEditor->editAggregation( 0 ); // forget it

  delete item; // this will trigger aggregationListCurrentItemChanged()
  mDeleteAggregationButton->setEnabled( item && !item->aggregation()->readOnly() );
}

void ConfigureAggregationsDialog::Private::importAggregationButtonClicked()
{
//TODO
}

void ConfigureAggregationsDialog::Private::exportAggregationButtonClicked()
{
//TODO
}


#include "configureaggregationsdialog.moc"
