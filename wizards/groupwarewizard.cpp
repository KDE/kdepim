/*
    This file is part of kdepim.

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

#include <qapplication.h>
#include <qlayout.h>

#include <klocale.h>

#include "actionpage.h"
#include "overviewpage.h"

#include "groupwarewizard.h"

GroupwareWizard::GroupwareWizard( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setCaption( i18n( "KDE Groupware Wizard" ) );

  QVBoxLayout *layout = new QVBoxLayout( this );

  mActionPage = new ActionPage( this );
  mActionPage->hide();

  mOverViewPage = new OverViewPage( this );
  mOverViewPage->hide();

  layout->addWidget( mActionPage );
  layout->addWidget( mOverViewPage );

  connect( mOverViewPage, SIGNAL( serverTypeSelected( const QString& ) ),
           this, SLOT( setServerType( const QString& ) ) );
  connect( mOverViewPage, SIGNAL( cancel() ),
           qApp, SLOT( quit() ) );

  resize( 400, 200 );
}

GroupwareWizard::~GroupwareWizard()
{
}

void GroupwareWizard::setServerType( const QString& serverType )
{
  if ( serverType.isEmpty() ) {
    mActionPage->hide();
    mOverViewPage->show();
  } else {
    mOverViewPage->hide();
    mActionPage->show();
    mActionPage->setServerType( serverType );
  }
}

#include "groupwarewizard.moc"
