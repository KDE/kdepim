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

#include <klocale.h>
#include <kdebug.h>
#include <kconfigskeleton.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <tqlistview.h>
#include <tqlayout.h>
#include <tqtimer.h>

KConfigWizard::KConfigWizard( TQWidget *parent,
                              char *name, bool modal )
  : KDialogBase( TreeList, i18n("Configuration Wizard"), Ok|Cancel, Ok, parent,
                 name, modal ),
    mPropagator( 0 ), mChangesPage( 0 )
{
  init();
}

KConfigWizard::KConfigWizard( KConfigPropagator *propagator, TQWidget *parent,
                              char *name, bool modal )
  : KDialogBase( TreeList, i18n("Configuration Wizard"), Ok|Cancel, Ok, parent,
                 name, modal ),
    mPropagator( propagator ), mChangesPage( 0 )
{
  init();
}

KConfigWizard::~KConfigWizard()
{
  delete mPropagator;
}

void KConfigWizard::init()
{
  connect( this, TQT_SIGNAL( aboutToShowPage( TQWidget * ) ),
           TQT_SLOT( slotAboutToShowPage( TQWidget * ) ) );

  TQTimer::singleShot( 0, this, TQT_SLOT( readConfig() ) );
}

void KConfigWizard::setPropagator( KConfigPropagator *p )
{
  mPropagator = p;
}

void KConfigWizard::slotAboutToShowPage( TQWidget *page )
{
  if ( page == mChangesPage ) {
    updateChanges();
  }
}

TQFrame *KConfigWizard::createWizardPage( const TQString &title )
{
  return addPage( title );
}

void KConfigWizard::setupRulesPage()
{
  TQFrame *topFrame = addPage( i18n("Rules") );
  TQVBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mRuleView = new TQListView( topFrame );
  topLayout->addWidget( mRuleView );

  mRuleView->addColumn( i18n("Source") );
  mRuleView->addColumn( i18n("Target") );
  mRuleView->addColumn( i18n("Condition") );

  updateRules();
}

void KConfigWizard::updateRules()
{
  if ( !mPropagator ) {
    kdError() << "KConfigWizard: No KConfigPropagator set." << endl;
    return;
  }

  mRuleView->clear();

  KConfigPropagator::Rule::List rules = mPropagator->rules();
  KConfigPropagator::Rule::List::ConstIterator it;
  for( it = rules.begin(); it != rules.end(); ++it ) {
    KConfigPropagator::Rule r = *it;
    TQString source = r.sourceFile + "/" + r.sourceGroup + "/" +
                     r.sourceEntry;
    TQString target = r.targetFile + "/" + r.targetGroup + "/" +
                     r.targetEntry;
    TQString condition;
    KConfigPropagator::Condition c = r.condition;
    if ( c.isValid ) {
      condition = c.file + "/" + c.group + "/" + c.key + " = " + c.value;
    }
    new TQListViewItem( mRuleView, source, target, condition );
  }
}

void KConfigWizard::setupChangesPage()
{
  TQFrame *topFrame = addPage( i18n("Changes") );
  TQVBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mChangeView = new TQListView( topFrame );
  topLayout->addWidget( mChangeView );

  mChangeView->addColumn( i18n("Action") );
  mChangeView->addColumn( i18n("Option") );
  mChangeView->addColumn( i18n("Value") );
  mChangeView->setSorting( -1 );

  mChangesPage = topFrame;
}

void KConfigWizard::updateChanges()
{
  kdDebug() << "KConfigWizard::updateChanges()" << endl;

  if ( !mPropagator ) {
    kdError() << "KConfigWizard: No KConfigPropagator set." << endl;
    return;
  }

  usrWriteConfig();

  mPropagator->updateChanges();

  mChangeView->clear();

  KConfigPropagator::Change::List changes = mPropagator->changes();
  KConfigPropagator::Change *c;
  for( c = changes.first(); c; c = changes.next() ) {
    new TQListViewItem( mChangeView, mChangeView->lastItem(), c->title(), c->arg1(), c->arg2() );
  }
}

void KConfigWizard::readConfig()
{
  kdDebug() << "KConfigWizard::readConfig()" << endl;

  int result = KMessageBox::warningContinueCancel( this,
      i18n("Please make sure that the programs which are "
           "configured by the wizard do not run in parallel to the wizard; "
           "otherwise, changes done by the wizard could be lost."),
      i18n("Warning"), i18n("Run Wizard Now"), "warning_running_instances" );
  if ( result != KMessageBox::Continue ) kapp->quit();

  usrReadConfig();
}

void KConfigWizard::slotOk()
{
  TQString error = validate();
  if ( error.isNull() ) {
    usrWriteConfig();

    if ( !mPropagator ) {
      kdError() << "KConfigWizard: No KConfigPropagator set." << endl;
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
