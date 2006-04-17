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

#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QGridLayout>

#include <klocale.h>

#include "kaddressbookview.h"

#include "addviewdialog.h"

AddViewDialog::AddViewDialog( Q3Dict<ViewFactory> *viewFactoryDict,
                              QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Add View" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name ),
   mViewFactoryDict( viewFactoryDict )
{
  mTypeId = 0;

  QWidget *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 2, 2 );
  layout->setSpacing( spacingHint() );
  layout->setRowStretch( 1, 1 );
  layout->setColStretch( 1, 1 );

  QLabel *label = new QLabel( i18n( "View name:" ), page );
  layout->addWidget( label, 0, 0 );

  mViewNameEdit = new QLineEdit( page );
  connect( mViewNameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  layout->addWidget( mViewNameEdit, 0, 1 );

  mTypeGroup = new Q3ButtonGroup( 0, Qt::Horizontal, i18n( "View Type" ), page );
  connect( mTypeGroup, SIGNAL( clicked( int ) ), this, SLOT( clicked( int ) ) );
  layout->addMultiCellWidget( mTypeGroup, 1, 1, 0, 1 );
  QGridLayout *groupLayout = new QGridLayout( mTypeGroup->layout(), 3, 2 );
  groupLayout->setSpacing( spacingHint() );

  int row = 0;
  Q3DictIterator<ViewFactory> iter( *mViewFactoryDict );
  for ( iter.toFirst(); iter.current(); ++iter ) {
    QRadioButton *button = new QRadioButton( i18n((*iter)->type().toUtf8()),
                                             mTypeGroup, (*iter)->type().toLatin1() );
    label = new QLabel( (*iter)->description(), mTypeGroup );
    label->setWordWrap( true );

    groupLayout->addWidget( button, row, 0, Qt::AlignTop );
    groupLayout->addWidget( label, row, 1, Qt::AlignTop );

    row++;
  }

  mTypeGroup->setButton( 0 );
  mViewNameEdit->setFocus();
  enableButton( KDialogBase::Ok, false );
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
  return mTypeGroup->find( mTypeId )->name();
}

void AddViewDialog::clicked( int id )
{
  mTypeId = id;
}

void AddViewDialog::textChanged( const QString &text )
{
  enableButton( KDialogBase::Ok, !text.isEmpty() );
}

#include "addviewdialog.moc"
