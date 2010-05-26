/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "eventortododialog.h"

#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QTabWidget>

#include <KLocale>

#include <KCal/Event>
#include <KCal/Todo>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/Item>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "incidenceeditorgeneralpage.h"
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

using namespace Akonadi;
using namespace IncidenceEditorsNG;

EventOrTodoDialog::EventOrTodoDialog( QWidget *parent )
  : KDialog( parent )
  , mCalSelector( new Akonadi::CollectionComboBox( mainWidget() ) )
  , mGeneralPage( new IncidenceEditorGeneralPage )
{
  // Calendar selector
  mCalSelector->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
  //mCalSelector->setDefaultCollection( KCalPrefs::instance()->defaultCollection() );
  
  QLabel *callabel = new QLabel( i18n( "Calendar:" ), mainWidget() );
  callabel->setBuddy( mCalSelector );
  
  QHBoxLayout *callayout = new QHBoxLayout;
  callayout->setSpacing( KDialog::spacingHint() );
  callayout->addWidget( callabel );
  callayout->addWidget( mCalSelector, 1 );

  // Tab widget and pages
  QTabWidget *tabWidget = new QTabWidget( mainWidget() );
  
  tabWidget->addTab( mGeneralPage, i18nc( "@title:tab general event settings", "&General" ) );

  // Overall layout of the complete dialog
  QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
  layout->setMargin( 0 );
  layout->setSpacing( 0 );
  layout->addLayout( callayout );
  layout->addWidget( tabWidget );

  mainWidget()->setLayout( layout );
  setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel );
  setDefaultButton( Ok );
  enableButton( Ok, false );
  enableButton( Apply, false );
  setModal( false );
  showButtonSeparator( false );
}

void EventOrTodoDialog::load( const Akonadi::Item &item )
{
  if ( item.id() < 0 ) {
    // We're creating a new item
    Q_ASSERT( item.hasPayload() );
    Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
    Q_ASSERT( item.hasPayload<KCal::Event::Ptr>() || item.payload<KCal::Todo::Ptr>() );

    KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
    setCaption( i18nc( "@title:window",
                     "New %1", QString( incidence->type() ) ) );
    mGeneralPage->load( incidence );
    show();
  } else if ( item.hasPayload() ) {
    
    if ( item.hasPayload<KCal::Event::Ptr>() ) {
      mCalSelector->setMimeTypeFilter(
        QStringList() << IncidenceMimeTypeVisitor::eventMimeType() );
    } else {
      mCalSelector->setMimeTypeFilter(
        QStringList() << IncidenceMimeTypeVisitor::todoMimeType() );
    }
    
    KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
    setCaption( i18nc( "@title:window",
                     "Edit %1: %2", QString( incidence->type() ), incidence->summary() ) );
    mGeneralPage->load( incidence );
  } else {
    ItemFetchJob *job = new ItemFetchJob( item, this );
    job->fetchScope().fetchFullPayload();;

    connect( job, SIGNAL(result(KJob*)), SLOT(itemFetchResult(KJob*)) );
    return;
  }

  if ( item.hasPayload<KCal::Event::Ptr>() )
    mCalSelector->setMimeTypeFilter(
      QStringList() << IncidenceMimeTypeVisitor::eventMimeType() );
  else
    mCalSelector->setMimeTypeFilter(
      QStringList() << IncidenceMimeTypeVisitor::todoMimeType() );

  mCalSelector->setEnabled( mCalSelector->count() > 1 );
}


/// private slots

void EventOrTodoDialog::itemFetchResult( KJob *job )
{
  Q_ASSERT( job );

  if ( job->error() ) {
    kDebug() << "ItemFetch failed" << job->errorString();
    reject();
    return;
  }

  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( fetchJob->items().isEmpty() ) {
    kDebug() << "No items returned by fetch job";
    reject();
    return;
  }

  Item item = fetchJob->items().first();
  if ( item.hasPayload<KCal::Event::Ptr>() || item.payload<KCal::Todo::Ptr>() ) {
    load( item );
    show();
  } else {
    kDebug() << "Item as invalid payload type";
    reject();
  }
}
