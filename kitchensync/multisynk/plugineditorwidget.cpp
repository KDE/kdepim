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
#include <klocale.h>
#include <kresources/configdialog.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "konnectorpair.h"
#include "plugineditorwidget.h"


PluginEditorWidget::PluginEditorWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  initGUI();

  connect( mTypeBox, SIGNAL( activated( int ) ), SLOT( typeChanged( int ) ) );
  connect( mOptionButton, SIGNAL( clicked() ), SLOT( changeOptions() ) );
}

PluginEditorWidget::~PluginEditorWidget()
{
}

void PluginEditorWidget::setLabel( const QString &label )
{
  mLabel->setText( label );
}

void PluginEditorWidget::set( KonnectorPair *pair, KSync::Konnector *konnector )
{
  mPair = pair;
  mOldKonnector = mKonnector = konnector;

  fillTypeBox();

  if ( mKonnector ) {
    QStringList types = mPair->manager()->resourceTypeNames();
    int pos = types.findIndex( mKonnector->type() );

    mTypeBox->setCurrentItem( pos );
  }
}

void PluginEditorWidget::get( KonnectorPair *pair )
{
  if ( mKonnector == mOldKonnector ) {
    if ( mKonnector )
      pair->manager()->change( mKonnector );
  } else {
    pair->manager()->remove( mOldKonnector );
    pair->manager()->add( mKonnector );
  }
}

void PluginEditorWidget::fillTypeBox()
{
  mTypeBox->clear();
  mTypeBox->insertStringList( mPair->manager()->resourceTypeDescriptions() );
}

void PluginEditorWidget::typeChanged( int )
{
  KSync::Konnector *konnector = mPair->manager()->createResource( currentType() );
  if ( konnector )
    mKonnector = konnector;
}

void PluginEditorWidget::changeOptions()
{
  if ( mKonnector == 0 )
    return;

  KRES::ConfigDialog dlg( this, "konnector", mKonnector );

  dlg.exec();
}

QString PluginEditorWidget::currentType() const
{
  return mPair->manager()->resourceTypeNames()[ mTypeBox->currentItem() ];
}

void PluginEditorWidget::initGUI()
{
  QGridLayout *layout = new QGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mLabel = new QLabel( this );
  layout->addWidget( mLabel, 0, 0 );

  mTypeBox = new KComboBox( this );
  layout->addWidget( mTypeBox, 0, 1 );

  mLabel->setBuddy( mTypeBox );

  mOptionButton = new QPushButton( i18n( "Options..." ), this );
  layout->addWidget( mOptionButton, 0, 2 );

  mInfoLabel = new QLabel( this );
  layout->addMultiCellWidget( mInfoLabel, 1, 1, 1, 2 );
}

#include "plugineditorwidget.moc"
