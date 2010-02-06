/*
  This file is part of libkdepim.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kconfigwizard.h"
#include "kconfigpropagator.h"

#include <KAboutData>
#include <KComponentData>
#include <KConfigSkeleton>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>
#include <KPageDialog>

#include <QApplication>
#include <QLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QTreeWidget>

using namespace KPIM;

KConfigWizard::KConfigWizard( QWidget *parent, bool modal )
  : KPageDialog( parent ), mPropagator( 0 ), mChangesPage( 0 )
{
  setModal( modal );
  init();
}

KConfigWizard::KConfigWizard( KConfigPropagator *propagator, QWidget *parent,
                              bool modal )
  : KPageDialog( parent ), mPropagator( propagator ), mChangesPage( 0 )
{
  setModal( modal );
  init();
}

KConfigWizard::~KConfigWizard()
{
  delete mPropagator;
}

void KConfigWizard::init()
{
  setFaceType( KPageDialog::Tree );
  setWindowTitle(
    KGlobal::mainComponent().aboutData()->programName().isEmpty()
    ? i18nc( "@title:window", "Configuration Wizard" )
    : KGlobal::mainComponent().aboutData()->programName() );
  setWindowIcon( KIcon("tools-wizard") );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  connect( this, SIGNAL( currentPageChanged(KPageWidgetItem *, KPageWidgetItem * )),
           SLOT( slotAboutToShowPage(KPageWidgetItem *, KPageWidgetItem *) ) );
  connect( this, SIGNAL(okClicked()),
           SLOT( slotOk()));
  QTimer::singleShot( 0, this, SLOT( readConfig() ) );
}

void KConfigWizard::setPropagator( KConfigPropagator *p )
{
  mPropagator = p;
}

void KConfigWizard::slotAboutToShowPage( KPageWidgetItem *page, KPageWidgetItem * )
{
  if ( page == mChangesPage ) {
    updateChanges();
  }
}

QWidget *KConfigWizard::createWizardPage( const QString &title )
{
  QFrame *page = new QFrame(this);
  addPage( page, title );
  return page;
}

void KConfigWizard::setupRulesPage()
{
  QFrame *page = new QFrame(this);
  KPageWidgetItem *item = addPage( page, i18nc( "@title:tab", "Rules" ) );
  item->setHeader( i18nc( "@title:window", "Set Up Rules" ) );
  //TODO: set item icon
  //rame *topFrame = new QFrame( this );
  QVBoxLayout *topLayout = new QVBoxLayout;
  page->setLayout(topLayout);
  mRuleView = new QTreeWidget;
  topLayout->addWidget( mRuleView );

  mRuleView->setHeaderLabels( QStringList()
   << i18nc( "@title:column source file,group,entry", "Source" )
   << i18nc( "@title:column target file,group,entry", "Target" )
   << i18nc( "@title:column file,group,key,value", "Condition" ) );

  updateRules();
}

void KConfigWizard::updateRules()
{
  if ( !mPropagator ) {
    kError() << "KConfigWizard: No KConfigPropagator set.";
    return;
  }

  mRuleView->clear();

  const KConfigPropagator::Rule::List rules = mPropagator->rules();
  KConfigPropagator::Rule::List::ConstIterator it;
  for ( it = rules.constBegin(); it != rules.constEnd(); ++it ) {
    KConfigPropagator::Rule r = *it;
    QString source = r.sourceFile + '/' + r.sourceGroup + '/' +
                     r.sourceEntry;
    QString target = r.targetFile + '/' + r.targetGroup + '/' +
                     r.targetEntry;
    QString condition;
    KConfigPropagator::Condition c = r.condition;
    if ( c.isValid ) {
      condition = c.file + '/' + c.group + '/' + c.key + " = " + c.value;
    }
    QTreeWidgetItem *item = new QTreeWidgetItem( mRuleView );
    item->setText( 0, source );
    item->setText( 1, target );
    item->setText( 2, condition );
  }
}

void KConfigWizard::setupChangesPage()
{
  QFrame *page = new QFrame( this );
  KPageWidgetItem *item = addPage( page, i18nc( "@title:tab", "Changes" ) );
  item->setHeader( i18nc( "@title:window", "Set Up Changes" ) );
  //TODO: set item icon
  QVBoxLayout *topLayout = new QVBoxLayout;
  page->setLayout(topLayout);
  mChangeView = new QTreeWidget;
  topLayout->addWidget( mChangeView );

  mChangeView->setHeaderLabels( QStringList() 
    << i18nc( "@title:column change action", "Action" )
    << i18nc( "@title:column option to change", "Option" )
    << i18nc( "@title:column value for option", "Value" ) );
  mChangeView->setSortingEnabled( false );

  mChangesPage = item;
}

void KConfigWizard::updateChanges()
{
  kDebug() << "KConfigWizard::updateChanges()";

  if ( !mPropagator ) {
    kError() << "KConfigWizard: No KConfigPropagator set.";
    return;
  }

  usrWriteConfig();

  mPropagator->updateChanges();

  mChangeView->clear();

  foreach( KConfigPropagator::Change *c, mPropagator->changes() ) {
    QTreeWidgetItem *item = new QTreeWidgetItem( mChangeView );
    item->setText( 0, c->title() );
    item->setText( 1, c->arg1() );
    item->setText( 2, c->arg2() );
  }
}

void KConfigWizard::readConfig()
{
  kDebug() << "KConfigWizard::readConfig()";

  setEnabled( false );
  int result = KMessageBox::warningContinueCancel(
    0,
    i18nc( "@info", "Please make sure that the programs which are "
          "configured by the wizard do not run in parallel to the wizard; "
          "otherwise, changes done by the wizard could be lost." ),
    i18nc( "@title:window warn about running instances", "Warning" ),
    KGuiItem( i18nc( "@action:button", "Run Wizard Now" ) ),
    KStandardGuiItem::cancel(), "warning_running_instances" );
  if ( result != KMessageBox::Continue ) {
    qApp->quit();
  }
  setEnabled( true );

  usrReadConfig();
}

void KConfigWizard::slotOk()
{
  QString error = validate();
  if ( error.isNull() ) {
    usrWriteConfig();

    if ( !mPropagator ) {
      kError() << "KConfigWizard: No KConfigPropagator set.";
      return;
    } else {
      if ( mPropagator->skeleton() ) {
        mPropagator->skeleton()->writeConfig();
      }
      mPropagator->commit();
    }

    accept();
  } else {
    KMessageBox::sorry( this, error );
  }
}

#include "kconfigwizard.moc"
