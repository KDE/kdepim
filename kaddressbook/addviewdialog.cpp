/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>

#include <KLocale>

#include "kaddressbookview.h"

#include "addviewdialog.h"

AddViewDialog::AddViewDialog( QHash<QString, ViewFactory*> *viewFactoryDict,
                              QWidget *parent, const char *name )
  : KDialog( parent),
   mViewFactoryDict( viewFactoryDict )
{
  setCaption( i18n( "Add View" ) );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setDefaultButton( KDialog::Ok );

  mTypeId = 0;

  QWidget *page = new QWidget(this);
  setMainWidget( page );

  QGridLayout *layout = new QGridLayout( page );
  layout->setMargin( 0 );
  layout->setSpacing( spacingHint() );
  layout->setRowStretch( 1, 1 );
  layout->setColumnStretch( 1, 1 );

  QLabel *label = new QLabel( i18n( "View name:" ), page );
  layout->addWidget( label, 0, 0 );

  mViewNameEdit = new QLineEdit( page );
  connect( mViewNameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  layout->addWidget( mViewNameEdit, 0, 1 );

  QGroupBox *group = new QGroupBox( i18n( "View Type" ), page );
  mTypeGroup = new QButtonGroup;
  mTypeGroup->setExclusive( true );
  connect( mTypeGroup, SIGNAL( buttonClicked( int ) ), 
           this, SLOT( clicked( int ) ) );
  layout->addWidget( group, 1, 0, 1, 2 );
  QGridLayout *groupLayout = new QGridLayout();
  groupLayout->setMargin( KDialog::marginHint() );
  groupLayout->setSpacing( KDialog::spacingHint() );
  group->setLayout( groupLayout );

  int row = 0;
  QHashIterator<QString, ViewFactory*> iter( *mViewFactoryDict );
  while ( iter.hasNext() ) {
    iter.next();
    QRadioButton *button = new QRadioButton( i18n( iter.value()->type().toUtf8() ),
                                             group );
    button->setObjectName( iter.value()->type().toLatin1() );
    mTypeGroup->addButton( button, row );
    label = new QLabel( iter.value()->description(), group );
    label->setWordWrap( true );

    groupLayout->addWidget( button, row, 0, Qt::AlignTop );
    groupLayout->addWidget( label, row, 1, Qt::AlignTop );

    row++;
  }

  mTypeGroup->button( 0 )->setChecked( true );
  mViewNameEdit->setFocus();
  enableButton( KDialog::Ok, false );
}

AddViewDialog::~AddViewDialog()
{
}

QString AddViewDialog::viewName()const
{
  return mViewNameEdit->text();
}

QString AddViewDialog::viewType()const
{
  // we missuse the name property for storing the type
  return mTypeGroup->button( mTypeId )->objectName();
}

void AddViewDialog::clicked( int id )
{
  mTypeId = id;
}

void AddViewDialog::textChanged( const QString &text )
{
  enableButton( KDialog::Ok, !text.isEmpty() );
}

#include "addviewdialog.moc"
