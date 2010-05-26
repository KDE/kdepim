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

#include <Akonadi/CollectionComboBox>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include "incidenceeditorgeneralpage.h"

using namespace IncidenceEditorsNG;

EventOrTodoDialog::EventOrTodoDialog( QWidget *parent )
  : KDialog( parent )
{
  // Calendar selector
  Akonadi::CollectionComboBox *mCalSelector = new Akonadi::CollectionComboBox( mainWidget() );
  mCalSelector->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
  //mCalSelector->setDefaultCollection( KCalPrefs::instance()->defaultCollection() );
  //mCalSelector->addExcludeResourcesType(QStringList()<<"akonadi_search_resource");
  mCalSelector->setMimeTypeFilter(
    QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  
  QLabel *callabel = new QLabel( i18n( "Calendar:" ), mainWidget() );
  callabel->setBuddy( mCalSelector );
  
  QHBoxLayout *callayout = new QHBoxLayout;
  callayout->setSpacing( KDialog::spacingHint() );
  callayout->addWidget( callabel );
  callayout->addWidget( mCalSelector, 1 );

  // Tab widget and pages
  QTabWidget *tabWidget = new QTabWidget( mainWidget() );
  IncidenceEditorGeneralPage *generalPage = new IncidenceEditorGeneralPage( tabWidget );
  tabWidget->addTab( generalPage, i18nc( "@title:tab general event settings", "&General" ) );

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

