/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>

#include "konnectorpair.h"
#include "plugineditorwidget.h"

#include "paireditorwidget.h"

PairEditorWidget::PairEditorWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  initGUI();
}

PairEditorWidget::~PairEditorWidget()
{
}

void PairEditorWidget::setPair( KonnectorPair *pair )
{
  mPair = pair;

  mPairNameEdit->setText( mPair->name() );

  switch ( mPair->resolveStrategy() ) {
    case KonnectorPair::ResolveManually:
      mResolveManually->setChecked( true );
      break;
    case KonnectorPair::ResolveFirst:
      mResolveFirst->setChecked( true );
      break;
    case KonnectorPair::ResolveSecond:
      mResolveSecond->setChecked( true );
      break;
    case KonnectorPair::ResolveBoth:
      mResolveBoth->setChecked( true );
      break;
  }

  KonnectorManager *manager = mPair->manager();
  KonnectorManager::Iterator it = manager->begin();

  KSync::Konnector *konnector;
  if ( it != manager->end() )
    konnector = *it;
  else
    konnector = 0;
  it++;

  mEditorWidgets[ 0 ]->set( mPair, konnector );

  if ( it != manager->end() )
    konnector = *it;
  else
    konnector = 0;

  mEditorWidgets[ 1 ]->set( mPair, konnector );
}

KonnectorPair *PairEditorWidget::pair() const
{
  mPair->setName( mPairNameEdit->text() );
  mEditorWidgets[ 0 ]->get( mPair );
  mEditorWidgets[ 1 ]->get( mPair );

  if ( mResolveManually->isChecked() )
    mPair->setResolveStrategy( KonnectorPair::ResolveManually );
  else if ( mResolveFirst->isChecked() )
    mPair->setResolveStrategy( KonnectorPair::ResolveFirst );
  else if ( mResolveSecond->isChecked() )
    mPair->setResolveStrategy( KonnectorPair::ResolveSecond );
  else if ( mResolveBoth->isChecked() )
    mPair->setResolveStrategy( KonnectorPair::ResolveBoth );

  return mPair;
}

void PairEditorWidget::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QTabWidget *tabWidget = new QTabWidget( this );
  layout->addWidget( tabWidget );

  tabWidget->addTab( createPluginTab(), i18n( "Plugins" ) );
  tabWidget->addTab( createSyncOptionTab(), i18n( "Synchronize Options" ) );
//  tabWidget->addTab( createFilterTab(), i18n( "Filters" ) );
}

QWidget *PairEditorWidget::createPluginTab()
{
  QWidget *widget = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( widget, KDialog::marginHint(), KDialog::spacingHint() );

  QLabel *label = new QLabel( "<h2><b>" + i18n( "Synchronization Plugins:" ) + "</b></h2>", widget );
  layout->addWidget( label );

  QVBoxLayout *pluginLayout = new QVBoxLayout( widget, KDialog::marginHint(), KDialog::spacingHint() );

  PluginEditorWidget *firstPlugin = new PluginEditorWidget( widget );
  firstPlugin->setLabel( i18n( "First Plugin:" ) );

  PluginEditorWidget *secondPlugin = new PluginEditorWidget( widget );
  secondPlugin->setLabel( i18n( "Second Plugin:" ) );

  mEditorWidgets.append( firstPlugin );
  mEditorWidgets.append( secondPlugin );

  pluginLayout->addWidget( firstPlugin );
  pluginLayout->addWidget( secondPlugin );

  QHBoxLayout *displayLayout = new QHBoxLayout( widget, KDialog::marginHint(), KDialog::spacingHint() );
  label = new QLabel( i18n( "Display Name:" ), widget );
  displayLayout->addWidget( label );

  mPairNameEdit = new KLineEdit( widget );
  displayLayout->addWidget( mPairNameEdit );

  pluginLayout->addLayout( displayLayout );

  layout->addLayout( pluginLayout );

  layout->addStretch( 10 );  

  return widget;
}

QWidget *PairEditorWidget::createSyncOptionTab()
{
  QWidget *widget = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( widget, KDialog::marginHint(), KDialog::spacingHint() );

  QLabel *label = new QLabel( "<h2><b>" + i18n( "Conflicts and near duplicates:" ) + "</b></h2>", widget );
  layout->addWidget( label );

  QVBoxLayout *groupLayout = new QVBoxLayout( widget, KDialog::marginHint(), KDialog::spacingHint() );

  QButtonGroup *group = new QButtonGroup( 1, Qt::Horizontal, widget );
  group->setRadioButtonExclusive( true );

  mResolveManually = new QRadioButton( i18n( "Resolve it manually" ), group );
  mResolveFirst = new QRadioButton( i18n( "Always use the entry from the first plugin" ), group );
  mResolveSecond = new QRadioButton( i18n( "Always use the entry from the second plugin" ), group );
  mResolveBoth = new QRadioButton( i18n( "Always put both entries on both sides" ), group );

  groupLayout->addWidget( group );

  layout->addLayout( groupLayout );

  layout->addStretch( 10 );

  return widget;
}

QWidget *PairEditorWidget::createFilterTab()
{
  return new QWidget( this );
}

#include "paireditorwidget.moc"
